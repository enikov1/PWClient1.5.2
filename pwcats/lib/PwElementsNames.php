<?php

require_once __DIR__ . '/PwBinReader.php';
require_once __DIR__ . '/PwElementsCfgLayout.php';

/**
 * Loads addon / item names from elements.data (same order as elementdataman::load_data)
 * or from JSON cache built by build_elements_cache.php.
 *
 * Точные размеры записей и порядок «листов» должны совпадать с **вашим** собранным `gs`:
 * исходники exptypes.h и список массивов в elementdataman могут отличаться от шарового билда.
 * Сообщество часто снимает фактический layout так: `gdb /path/to/gamed/gs`, затем
 * `ptype elementdataman` (порядок add_structure / полей), `ptype EQUIPMENT_ADDON`, … для каждого типа;
 * вывод можно писать в файл (`set logging file …`, `set logging on`).
 * Экспортная схема **PW_\*.cfg** (ElementEditor / похожие тулзы) задаёт те же поля: подключите файл через
 * `PWCATS_ELEMENTS_CFG` или положите `PW_1.5.3_v145.cfg` в каталог `pwcats/` — тогда размеры **брони/декора/зелий**
 * и хвоста берутся из cfg (см. `PwElementsCfgLayout`), перебор armor→material отключается.
 * **WEAPON_SUB / WEAPON_ESSENCE** всегда подбираются по цепочке в `elements.data`: в cfg и exptypes они часто совпадают
 * с серверным билдом, но один отличающийся sizeof ломает весь парсинг до skilltome.
 * Заголовок файла в load_data — это **uint32_t version** и **time_t** (4 или 8 байт на целевой ОС),
 * а не short/short/int из старых гайдов. Перед каждым массивом — uint32_t count, далее count * sizeof(T).
 */
class PwElementsNames
{
    const EXPECTED_VERSION = 0x30000091;

    /** @var string последняя диагностика loadElementsBinary (для CLI) */
    private static $lastLoadDiag = '';

    /** @var int после авто-проверки цепочки weapon (по умолчанию 356) */
    private $weaponSubRecordSize = 356;

    /** @var int sizeof(WEAPON_ESSENCE), подбирается вместе с weapon_sub */
    private $weaponEssenceRecordSize = 1424;

    /** @var int sizeof(ARMOR_ESSENCE) — в exptypes этого дерева 1164; старый хардкод 1228 ломал выравнивание */
    private $armorEssenceRecordSize = 1164;

    /** @var int sizeof(DECORATION_ESSENCE) — по exptypes 1172 */
    private $decorationEssenceRecordSize = 1172;

    /** @var int sizeof(DECORATION_SUB_TYPE), обычно 72 */
    private $decorationSubRecordSize = 72;

    /** @var int sizeof(MEDICINE_SUB_TYPE), обычно 68 */
    private $medicineSubRecordSize = 68;

    /** @var int sizeof(MEDICINE_ESSENCE), обычно 376 */
    private $medicineEssenceRecordSize = 376;

    /** @var int sizeof(MATERIAL_ESSENCE) до exporter */
    private $materialEssenceRecordSize = 368;

    /** @var int sizeof(DAMAGERUNE_ESSENCE) */
    private $damageRuneEssenceRecordSize = 364;

    /** @var int sizeof(ARMORRUNE_ESSENCE) */
    private $armorRuneEssenceRecordSize = 624;

    /** @var int sizeof(STONE_ESSENCE) */
    private $stoneEssenceRecordSize = 476;

    /** @var int[]|null если задано из PW_*.cfg — skip после exporter */
    private $tailSkipsFromCfg = null;

    /** @var bool размеры цепочки equipment заданы из cfg, без эвристик */
    private $layoutAppliedFromCfg = false;

    /** @var array */
    private $addons = array();
    /** @var array string id => name */
    private $items = array();

    /** @var self|null */
    private static $inst;

    public static function instance()
    {
        if (self::$inst === null) {
            self::$inst = new self();
            self::$inst->load();
        }
        return self::$inst;
    }

    public static function resetInstance()
    {
        self::$inst = null;
    }

    public static function getLastLoadDiag()
    {
        return self::$lastLoadDiag;
    }

    public function getAddonName($pureId)
    {
        $k = (string)((int)$pureId);
        return isset($this->addons[$k]) ? $this->addons[$k] : null;
    }

    public function getItemName($itemId)
    {
        $k = (string)((int)$itemId);
        return isset($this->items[$k]) ? $this->items[$k] : null;
    }

    public function isLoaded()
    {
        return count($this->addons) > 0 || count($this->items) > 0;
    }

    public function getAddonsMap()
    {
        return $this->addons;
    }

    public function getItemsMap()
    {
        return $this->items;
    }

    /**
     * @param string $jsonPath
     * @param string $elementsPath optional raw elements.data
     */
    /**
     * @param string|null $jsonPath path to cache JSON, or '' to skip JSON and only read elements.data
     */
    public function load($jsonPath = null, $elementsPath = null)
    {
        $this->addons = array();
        $this->items = array();

        $skipJson = ($jsonPath === '');
        if ($jsonPath === null) {
            $jsonPath = getenv('PWCATS_ELEMENTS_JSON');
            if ($jsonPath === false || $jsonPath === '') {
                $jsonPath = __DIR__ . '/../data/elements_names.json';
            }
        }
        if ($elementsPath === null) {
            $elementsPath = getenv('PWCATS_ELEMENTS_DATA');
            if ($elementsPath === false || $elementsPath === '') {
                $elementsPath = '/root/pwserver/gamed/config/elements.data';
                if (!is_file($elementsPath)) {
                    $elementsPath = __DIR__ . '/../data/elements.data';
                }
            }
        }

        if (!$skipJson && is_string($jsonPath) && $jsonPath !== '' && is_file($jsonPath)) {
            $this->loadJson($jsonPath);
            if ($this->isLoaded()) {
                return;
            }
        }
        if (is_file($elementsPath)) {
            $this->loadElementsBinary($elementsPath);
        }
    }

    private function loadJson($path)
    {
        $t = json_decode((string)file_get_contents($path), true);
        if (!is_array($t)) {
            return;
        }
        if (isset($t['addons']) && is_array($t['addons'])) {
            foreach ($t['addons'] as $id => $row) {
                if (is_array($row) && isset($row['name'])) {
                    $this->addons[(string)$id] = (string)$row['name'];
                } elseif (is_string($row)) {
                    $this->addons[(string)$id] = $row;
                }
            }
        }
        if (isset($t['items']) && is_array($t['items'])) {
            foreach ($t['items'] as $id => $row) {
                if (is_array($row) && isset($row['name'])) {
                    $this->items[(string)$id] = (string)$row['name'];
                } elseif (is_string($row)) {
                    $this->items[(string)$id] = $row;
                }
            }
        }
    }

    /**
     * После version (u32) в elements.data идёт time_t: на билде может быть 4 или 8 байт.
     * Счётчик equipment_addon_array — всегда uint32 по elementdataman::array::load.
     */
    private function guessTimeSkipBytes($bin)
    {
        $l = strlen($bin);
        if ($l < 16) {
            return 8;
        }
        $u32 = function ($off) use ($bin, $l) {
            if ($off + 4 > $l) {
                return 0;
            }
            $p = unpack('V', substr($bin, $off, 4));
            return $p ? ($p[1] & 0xffffffff) : 0;
        };
        $nAt8 = $u32(8);
        $nAt12 = $u32(12);
        $plausible = function ($n) {
            return $n > 0 && $n <= 800000;
        };
        $ok8 = $plausible($nAt8);
        $ok12 = $plausible($nAt12);
        if ($ok8 && !$ok12) {
            return 4;
        }
        if ($ok12 && !$ok8) {
            return 8;
        }
        if ($ok8 && $ok12) {
            // При реальном time_t 4 байта: u32@8 = число записей, u32@12 = id первой записи —
            // оба часто одного порядка (тысячи) и НЕ отличаются в ~50 раз.
            $d = (int)abs((int)$nAt8 - (int)$nAt12);
            $mn = min((int)$nAt8, (int)$nAt12);
            if ($mn > 50 && $d < max(512, (int)($mn * 0.25))) {
                return 4;
            }
            if ($nAt12 > 200 && ($nAt8 < 5 || $nAt8 > $nAt12 * 50)) {
                return 8;
            }
            if ($nAt8 > 200 && ($nAt12 < 5 || $nAt12 > $nAt8 * 50)) {
                return 4;
            }
            return 4;
        }
        return 8;
    }

    /**
     * Struct sizes must match exptypes.h (pack 4, namechar = wchar_t 2 bytes).
     */
    private function loadElementsBinary($path)
    {
        self::$lastLoadDiag = '';
        $bin = file_get_contents($path);
        if ($bin === false || strlen($bin) < 16) {
            self::$lastLoadDiag = 'file missing or < 16 bytes';
            return;
        }
        $verU = unpack('V', substr($bin, 0, 4));
        $ver = $verU ? ($verU[1] & 0xffffffff) : 0;

        $hint = $this->guessTimeSkipBytes($bin);
        // Сначала 4-byte time_t (часто u32@12 — это id первой записи аддона, не count).
        $order = array(4, 8);
        self::$lastLoadDiag = sprintf(
            'bytes=%d version=0x%08X hint_time_skip=%d tries=%s',
            strlen($bin),
            $ver,
            $hint,
            implode(',', $order)
        );

        $lastEx = null;
        foreach ($order as $timeSkip) {
            try {
                $this->addons = array();
                $this->items = array();
                $r = new PwBinReader($bin);
                $r->u32(); // version
                $r->skip((int)$timeSkip);
                $this->parseElementsTail($r, $bin);
                if (count($this->addons) > 0) {
                    self::$lastLoadDiag .= sprintf('; OK time_skip=%d addons=%d items=%d', $timeSkip, count($this->addons), count($this->items));
                    return;
                }
                self::$lastLoadDiag .= sprintf('; time_skip=%d -> 0 addons', $timeSkip);
            } catch (Exception $e) {
                $lastEx = $e;
                self::$lastLoadDiag .= sprintf('; time_skip=%d EX:%s', $timeSkip, $e->getMessage());
                $this->addons = array();
                $this->items = array();
            }
        }
        if ($lastEx !== null) {
            self::$lastLoadDiag .= '; last_error=' . $lastEx->getMessage();
        }
        self::$lastLoadDiag .= '; head32=' . bin2hex(substr($bin, 0, 32));
    }

    /**
     * Смещение уже после version + time_t.
     *
     * @param string|null $fullBin весь файл — для автоопределения шага EQUIPMENT_ADDON
     */
    private function parseElementsTail(PwBinReader $r, $fullBin = null)
    {
        $this->resetTableLayoutDefaults();
        $this->tryApplyPwCfgLayout();
        $this->parseEquipmentAddons($r, $fullBin);
        if ($fullBin === null || $fullBin === '') {
            throw new InvalidArgumentException('parseElementsTail: full binary missing');
        }
        $wmStart = $r->tell();
        $wsRec = 0;
        $weRec = 0;
        if (!$this->tryResolveWeaponRows($fullBin, $wmStart, $wsRec, $weRec)) {
            throw new InvalidArgumentException(
                'weapon tables: cannot align weapon_sub / weapon_essence to armor_major; ' .
                'try PWCATS_WEAPON_SUB_REC / PWCATS_WEAPON_ESS_REC=<bytes> or fix header time_t (4 vs 8)'
            );
        }
        $this->weaponSubRecordSize = $wsRec;
        $this->weaponEssenceRecordSize = $weRec;
        self::$lastLoadDiag .= '; weapon_sub_rec=' . $this->weaponSubRecordSize . ',weapon_ess_rec=' . $this->weaponEssenceRecordSize;
        $this->assertReasonableArrayCountPeek($r, 'weapon_major_type');
        $this->skipSimpleArray($r, 68); // WEAPON_MAJOR_TYPE
        $this->skipSimpleArray($r, $this->weaponSubRecordSize); // WEAPON_SUB_TYPE
        $this->indexWeaponEssences($r);
        $this->skipSimpleArray($r, 68); // ARMOR_MAJOR
        $this->skipSimpleArray($r, 72); // ARMOR_SUB
        if (!$this->layoutAppliedFromCfg) {
            $this->resolveArmorThroughMaterialMajor($fullBin, $r->tell());
        }
        $this->indexArmorEssences($r);
        $this->skipSimpleArray($r, 68); // DECO_MAJOR
        $this->skipSimpleArray($r, $this->decorationSubRecordSize); // DECO_SUB
        $this->indexDecorationEssences($r);

        $this->skipSimpleArray($r, 68); // MED_MAJOR
        $this->skipSimpleArray($r, $this->medicineSubRecordSize); // MED_SUB
        $this->skipSimpleArray($r, $this->medicineEssenceRecordSize); // MED_ESS
        $this->skipSimpleArray($r, 68); // MAT_MAJOR
        $this->skipSimpleArray($r, 68); // MAT_SUB
        $this->skipSimpleArray($r, $this->materialEssenceRecordSize); // MAT_ESS
        $this->skipSimpleArray($r, 68); // DRUNE_SUB
        $this->skipSimpleArray($r, $this->damageRuneEssenceRecordSize); // DRUNE_ESS
        $this->skipSimpleArray($r, 68); // ARUNE_SUB
        $this->skipSimpleArray($r, $this->armorRuneEssenceRecordSize); // ARUNE_ESS

        $this->skipExporterBlock($r);

        if ($this->tailSkipsFromCfg !== null) {
            foreach ($this->tailSkipsFromCfg as $rec) {
                $this->skipSimpleArray($r, (int)$rec);
            }
        } else {
            $this->skipSimpleArray($r, 68);
            $this->skipSimpleArray($r, 348);
            $this->skipSimpleArray($r, 792);
            $this->skipSimpleArray($r, 488);
            $this->skipSimpleArray($r, 348);
            $this->skipSimpleArray($r, 348);
            $this->skipSimpleArray($r, 352);
            $this->skipSimpleArray($r, 356);
            $this->skipSimpleArray($r, 216);
            $this->skipSimpleArray($r, 888);
            $this->skipSimpleArray($r, 68);
            $this->skipSimpleArray($r, 892);
            $this->skipSimpleArray($r, 68);
            $this->skipSimpleArray($r, 336);
            $this->skipSimpleArray($r, 68);
        }
        $this->indexStoneEssences($r);
    }

    private function resetTableLayoutDefaults()
    {
        $this->weaponSubRecordSize = 356;
        $this->weaponEssenceRecordSize = 1424;
        $this->armorEssenceRecordSize = 1164;
        $this->decorationEssenceRecordSize = 1172;
        $this->decorationSubRecordSize = 72;
        $this->medicineSubRecordSize = 68;
        $this->medicineEssenceRecordSize = 376;
        $this->materialEssenceRecordSize = 368;
        $this->damageRuneEssenceRecordSize = 364;
        $this->armorRuneEssenceRecordSize = 624;
        $this->stoneEssenceRecordSize = 476;
        $this->tailSkipsFromCfg = null;
        $this->layoutAppliedFromCfg = false;
    }

    /**
     * PW_*.cfg из редактора элементов: фиксированные sizeof по строке типов.
     * PWCATS_ELEMENTS_CFG — явный путь; иначе, если есть pwcats/PW_1.5.3_v145.cfg, он подключается сам.
     */
    private function tryApplyPwCfgLayout()
    {
        $env = getenv('PWCATS_ELEMENTS_CFG');
        $path = null;
        if ($env !== false && $env !== '') {
            $path = $env;
            if (!is_file($path)) {
                throw new InvalidArgumentException('PWCATS_ELEMENTS_CFG file not found: ' . $path);
            }
        } else {
            $def = __DIR__ . '/../PW_1.5.3_v145.cfg';
            if (!is_file($def)) {
                return;
            }
            $path = $def;
        }
        $cfg = new PwElementsCfgLayout($path);
        $map = $cfg->getRecordSizesByName();
        $pick = function ($name) use ($map) {
            if (!isset($map[$name]) || !is_int($map[$name]) || $map[$name] <= 0) {
                return null;
            }
            return (int)$map[$name];
        };
        // Оружие из cfg не фиксируем: только эвристика по бинарнику (см. tryResolveWeaponRows).
        $chainKeys = array(
            'ARMOR_ESSENCE',
            'DECORATION_SUB_TYPE',
            'DECORATION_ESSENCE',
            'MEDICINE_SUB_TYPE',
            'MEDICINE_ESSENCE',
        );
        $chainOk = true;
        foreach ($chainKeys as $k) {
            if ($pick($k) === null) {
                $chainOk = false;
                break;
            }
        }
        self::$lastLoadDiag .= '; elements_cfg=' . basename($path) . ($chainOk ? '' : ' (armor chain incomplete, heuristics)');
        if ($chainOk) {
            $this->armorEssenceRecordSize = $pick('ARMOR_ESSENCE');
            $this->decorationSubRecordSize = $pick('DECORATION_SUB_TYPE');
            $this->decorationEssenceRecordSize = $pick('DECORATION_ESSENCE');
            $this->medicineSubRecordSize = $pick('MEDICINE_SUB_TYPE');
            $this->medicineEssenceRecordSize = $pick('MEDICINE_ESSENCE');
            $this->layoutAppliedFromCfg = true;
            self::$lastLoadDiag .= sprintf(
                '; armor_ess_rec=%d; deco_sub_rec=%d; decoration_ess_rec=%d; med_sub_rec=%d; med_ess_rec=%d',
                $this->armorEssenceRecordSize,
                $this->decorationSubRecordSize,
                $this->decorationEssenceRecordSize,
                $this->medicineSubRecordSize,
                $this->medicineEssenceRecordSize
            );
        }
        $m = $pick('MATERIAL_ESSENCE');
        if ($m !== null) {
            $this->materialEssenceRecordSize = $m;
        }
        $m = $pick('DAMAGERUNE_ESSENCE');
        if ($m !== null) {
            $this->damageRuneEssenceRecordSize = $m;
        }
        $m = $pick('ARMORRUNE_ESSENCE');
        if ($m !== null) {
            $this->armorRuneEssenceRecordSize = $m;
        }
        $m = $pick('STONE_ESSENCE');
        if ($m !== null) {
            $this->stoneEssenceRecordSize = $m;
        }
        $tailNames = PwElementsCfgLayout::tailTableNamesAfterExporter();
        $tailSizes = array();
        foreach ($tailNames as $tn) {
            $s = $pick($tn);
            if ($s === null) {
                $tailSizes = null;
                break;
            }
            $tailSizes[] = $s;
        }
        if ($tailSizes !== null && count($tailSizes) === count($tailNames)) {
            $this->tailSkipsFromCfg = $tailSizes;
        }
    }

    private function readU32At($bin, $off)
    {
        $len = strlen($bin);
        if ($off + 4 > $len) {
            return null;
        }
        $u = unpack('V', substr($bin, $off, 4));
        return $u ? ($u[1] & 0xffffffff) : null;
    }

    /**
     * @return int -100..120 выше — строка похожа на нормальный текст
     */
    private function textQualityScore($utf8)
    {
        if ($utf8 === null || $utf8 === '') {
            return 0;
        }
        $l = strlen($utf8);
        $bad = 0;
        for ($i = 0; $i < $l; $i++) {
            $c = ord($utf8[$i]);
            if ($c < 32 && $c !== 9 && $c !== 10 && $c !== 13) {
                $bad++;
            }
        }
        if ($l > 0 && $bad > (int)($l * 0.22)) {
            return -100;
        }
        return min(120, $l + ($bad === 0 ? 10 : 0));
    }

    private function mergeAddonEntry($pure, $name)
    {
        $k = (string)(int)$pure;
        $nq = $this->textQualityScore($name);
        if (!isset($this->addons[$k])) {
            if ($nq > -80) {
                $this->addons[$k] = $name;
            }
            return;
        }
        $oq = $this->textQualityScore($this->addons[$k]);
        if ($nq > $oq) {
            $this->addons[$k] = $name;
        }
    }

    private function mergeItemEntry($id, $name)
    {
        $id = (int)$id;
        if ($id === 0) {
            return;
        }
        $k = (string)$id;
        $nq = $this->textQualityScore($name);
        if (!isset($this->items[$k])) {
            if ($nq > -80) {
                $this->items[$k] = $name;
            }
            return;
        }
        $oq = $this->textQualityScore($this->items[$k]);
        if ($nq > $oq) {
            $this->items[$k] = $name;
        }
    }

    /**
     * Подбор согласованной цепочки от ARMOR_ESSENCE до MATERIAL_MAJOR:
     * размер строк брони, DECORATION_SUB, DECORATION_ESSENCE, MEDICINE_SUB, MEDICINE_ESSENCE.
     * Иначе при фиксированных 72/68 кастомный elements.data уезжает, как у вас на этапе зелий.
     *
     * @param int $pArmEssCount смещение u32 числа записей ARMOR_ESSENCE
     */
    private function resolveArmorThroughMaterialMajor($bin, $pArmEssCount)
    {
        $len = strlen($bin);
        $aeN = $this->readU32At($bin, $pArmEssCount);
        if ($aeN === null || $aeN < 1 || $aeN > 500000) {
            throw new InvalidArgumentException('armor_essence count bad at offset ' . $pArmEssCount);
        }

        $armorCands = $this->envIntListOrRange('PWCATS_ARMOR_ESS_REC', 1072, 1276, 4);
        $decoSubCands = $this->envIntListOrRange('PWCATS_DECO_SUB_REC', 60, 88, 4);
        $decoEssCands = $this->envIntListOrRange('PWCATS_DECORATION_ESS_REC', 1056, 1248, 4);
        $medSubCands = $this->envIntListOrRange('PWCATS_MED_SUB_REC', 60, 80, 4);
        $medEssCands = $this->envIntListOrRange('PWCATS_MED_ESS_REC', 356, 388, 4);

        $best = null;
        $bestScore = -1;
        $maj = 68;

        foreach ($armorCands as $aRec) {
            if ($aRec < 800 || $aRec > 1700) {
                continue;
            }
            $pDm = $pArmEssCount + 4 + (int)$aeN * $aRec;
            if ($pDm + 4 > $len) {
                continue;
            }
            $dm = $this->readU32At($bin, $pDm);
            if ($dm === null || $dm < 1 || $dm > 100000) {
                continue;
            }
            $pDs = $pDm + 4 + (int)$dm * $maj;
            if ($pDs + 4 > $len) {
                continue;
            }
            $ds = $this->readU32At($bin, $pDs);
            if ($ds === null || $ds < 1 || $ds > 500000) {
                continue;
            }
            foreach ($decoSubCands as $dSubRec) {
                $pDeCnt = $pDs + 4 + (int)$ds * $dSubRec;
                if ($pDeCnt + 4 > $len) {
                    continue;
                }
                $deN = $this->readU32At($bin, $pDeCnt);
                if ($deN === null || $deN < 1 || $deN > 500000) {
                    continue;
                }
                foreach ($decoEssCands as $dEssRec) {
                    if ($dEssRec < 800 || $dEssRec > 1600) {
                        continue;
                    }
                    $pMm = $pDeCnt + 4 + (int)$deN * $dEssRec;
                    if ($pMm + 4 > $len) {
                        continue;
                    }
                    $mm = $this->readU32At($bin, $pMm);
                    if ($mm === null || $mm < 1 || $mm > 100000) {
                        continue;
                    }
                    foreach ($medSubCands as $mSubRec) {
                        $pMs = $pMm + 4 + (int)$mm * $maj;
                        if ($pMs + 4 > $len) {
                            continue;
                        }
                        $ms = $this->readU32At($bin, $pMs);
                        if ($ms === null || $ms < 1 || $ms > 500000) {
                            continue;
                        }
                        foreach ($medEssCands as $mEssRec) {
                            if ($mEssRec < 300 || $mEssRec > 500) {
                                continue;
                            }
                            $pMeCnt = $pMs + 4 + (int)$ms * $mSubRec;
                            if ($pMeCnt + 4 > $len) {
                                continue;
                            }
                            $meN = $this->readU32At($bin, $pMeCnt);
                            if ($meN === null || $meN < 1 || $meN > 500000) {
                                continue;
                            }
                            $pMat = $pMeCnt + 4 + (int)$meN * $mEssRec;
                            if ($pMat + 4 > $len) {
                                continue;
                            }
                            $matMajN = $this->readU32At($bin, $pMat);
                            if ($matMajN === null || $matMajN < 1 || $matMajN > 100000) {
                                continue;
                            }
                            $pMatSub = $pMat + 4 + (int)$matMajN * $maj;
                            $matSubN = $this->readU32At($bin, $pMatSub);
                            $matSubOk = ($matSubN !== null && $matSubN >= 1 && $matSubN <= 500000);

                            $score = 800;
                            if ($aRec === 1164) {
                                $score += 220;
                            }
                            if ($aRec === 1232) {
                                $score += 180;
                            }
                            if ($dEssRec === 1172) {
                                $score += 200;
                            }
                            if ($dSubRec === 72) {
                                $score += 100;
                            }
                            if ($mSubRec === 68) {
                                $score += 100;
                            }
                            if ($mEssRec === 376) {
                                $score += 100;
                            }
                            if ($matSubOk) {
                                $score += 90;
                            }
                            $score -= (int)(min(abs($aRec - 1164), abs($aRec - 1232)) / 8);
                            $score -= (int)(abs($dEssRec - 1172) / 8);
                            $score -= abs($dSubRec - 72) * 3;
                            $score -= abs($mSubRec - 68) * 3;
                            $score -= (int)(abs($mEssRec - 376) / 4);

                            if ($score > $bestScore) {
                                $bestScore = $score;
                                $best = array(
                                    'armor' => $aRec,
                                    'decoSub' => $dSubRec,
                                    'decoEss' => $dEssRec,
                                    'medSub' => $mSubRec,
                                    'medEss' => $mEssRec,
                                );
                            }
                        }
                    }
                }
            }
        }

        if ($best === null) {
            throw new InvalidArgumentException(
                'cannot resolve armor→decoration→medicine→material; try env PWCATS_ARMOR_ESS_REC, ' .
                'PWCATS_DECO_SUB_REC, PWCATS_DECORATION_ESS_REC, PWCATS_MED_SUB_REC, PWCATS_MED_ESS_REC'
            );
        }

        $this->armorEssenceRecordSize = (int)$best['armor'];
        $this->decorationSubRecordSize = (int)$best['decoSub'];
        $this->decorationEssenceRecordSize = (int)$best['decoEss'];
        $this->medicineSubRecordSize = (int)$best['medSub'];
        $this->medicineEssenceRecordSize = (int)$best['medEss'];
        self::$lastLoadDiag .= sprintf(
            '; armor_ess_rec=%d; deco_sub_rec=%d; decoration_ess_rec=%d; med_sub_rec=%d; med_ess_rec=%d',
            $this->armorEssenceRecordSize,
            $this->decorationSubRecordSize,
            $this->decorationEssenceRecordSize,
            $this->medicineSubRecordSize,
            $this->medicineEssenceRecordSize
        );
    }

    /**
     * Одно число из env или диапазон [min..max] с шагом.
     *
     * @param string $envName
     * @param int $min
     * @param int $max
     * @param int $step
     * @return int[]
     */
    private function envIntListOrRange($envName, $min, $max, $step)
    {
        $e = getenv($envName);
        if ($e !== false && $e !== '' && ctype_digit((string)$e)) {
            $v = (int)$e;
            return array($v);
        }
        $out = array();
        for ($x = $min; $x <= $max; $x += $step) {
            $out[] = $x;
        }
        return $out;
    }

    /**
     * После имени экспортёра — fread(..., sizeof(time_t)) перед skilltome (elementdataman::load_data).
     * PWCATS_EXPORTER_TIMET_SKIP=4|8 переопределяет авто; без env выбирается по u32 сразу после поля time_t
     * (число записей SKILLTOME_SUB_TYPE должно быть в правдоподобном диапазоне).
     */
    private function skipExporterBlock(PwBinReader $r)
    {
        $r->u32(); // tag
        $len = $r->u32();
        if ($len > 1 << 20) {
            throw new InvalidArgumentException('exporter block too large');
        }
        $r->skip((int)$len);
        $pos = $r->tell();
        $ets = getenv('PWCATS_EXPORTER_TIMET_SKIP');
        if ($ets === '4' || $ets === '04') {
            $r->skip(4);
            self::$lastLoadDiag .= '; exporter_time_t=4(env)';
            return;
        }
        if ($ets === '8' || $ets === '08') {
            $r->skip(8);
            self::$lastLoadDiag .= '; exporter_time_t=8(env)';
            return;
        }
        $plausible = function ($n) {
            return $n !== null && $n > 0 && $n <= 500000;
        };
        $n4 = $r->peekU32At($pos + 4);
        $n8 = $r->peekU32At($pos + 8);
        $ok4 = $plausible($n4);
        $ok8 = $plausible($n8);
        if ($ok4 && !$ok8) {
            $skip = 4;
        } elseif (!$ok4 && $ok8) {
            $skip = 8;
        } elseif ($ok4 && $ok8) {
            $skip = 4;
        } else {
            $skip = 8;
        }
        $r->skip($skip);
        self::$lastLoadDiag .= sprintf(
            '; exporter_time_t=%d(auto u32@+4=%s @+8=%s)',
            $skip,
            $n4 === null ? 'na' : (string)$n4,
            $n8 === null ? 'na' : (string)$n8
        );
    }

    private function skipSimpleArray(PwBinReader $r, $recSize)
    {
        $n = $r->u32();
        if ($n > 10000000) {
            throw new InvalidArgumentException('array count absurd (rec=' . $recSize . ', n=' . $n . ')');
        }
        $r->skip((int)$n * $recSize);
    }

    /** Проверка, что после массива аддонов читается правдоподобный count следующей таблицы. */
    private function assertReasonableArrayCountPeek(PwBinReader $r, $tableLabel)
    {
        $n = $r->peekU32();
        $max = 500000;
        if ($n > $max) {
            throw new InvalidArgumentException($tableLabel . ' peek_u32=' . $n . ' (>' . $max . ') misaligned?');
        }
    }

    /**
     * Подбор sizeof(WEAPON_SUB_TYPE) и sizeof(WEAPON_ESSENCE) так, чтобы после блока эссенций
     * на месте ARMOR_MAJOR_TYPE оказалось правдоподобное число записей (и при возможности ARMOR_SUB).
     * Иначе возможен «первый попавшийся» wsRec с валидным weapon_essence count — и сдвиг, как у вас:
     * array count absurd (rec=72, n=3858759680).
     *
     * @param int $wmBlockStart смещение u32 weapon_major count
     * @param int $wsRec output
     * @param int $weRec output
     */
    private function tryResolveWeaponRows($bin, $wmBlockStart, &$wsRec, &$weRec)
    {
        $len = strlen($bin);
        $wsRec = 0;
        $weRec = 0;
        if ($wmBlockStart + 4 > $len) {
            return false;
        }
        $u = unpack('V', substr($bin, $wmBlockStart, 4));
        $wm = $u ? ($u[1] & 0xffffffff) : 0;
        if ($wm < 1 || $wm > 500000) {
            return false;
        }
        $pWs = $wmBlockStart + 4 + (int)$wm * 68;
        if ($pWs + 4 > $len) {
            return false;
        }
        $u = unpack('V', substr($bin, $pWs, 4));
        $wsN = $u ? ($u[1] & 0xffffffff) : 0;
        if ($wsN < 1 || $wsN > 500000) {
            return false;
        }

        $wsCands = array();
        for ($s = 320; $s <= 448; $s += 4) {
            $wsCands[] = $s;
        }
        $wsCands = array_values(array_unique($wsCands));
        usort($wsCands, function ($a, $b) {
            $da = abs($a - 356);
            $db = abs($b - 356);
            if ($da !== $db) {
                return $da - $db;
            }
            return $a - $b;
        });

        $weCands = array();
        $envWe = getenv('PWCATS_WEAPON_ESS_REC');
        if ($envWe !== false && $envWe !== '' && ctype_digit((string)$envWe)) {
            $weCands[] = (int)$envWe;
        }
        for ($w = 1312; $w <= 1720; $w += 4) {
            $weCands[] = $w;
        }
        $weCands = array_values(array_unique($weCands));
        usort($weCands, function ($a, $b) {
            $da = abs($a - 1424);
            $db = abs($b - 1424);
            if ($da !== $db) {
                return $da - $db;
            }
            return $a - $b;
        });

        $bestWs = null;
        $bestWe = null;
        $bestScore = -1;
        $amMax = 100000;

        foreach ($wsCands as $wsR) {
            $pWe = $pWs + 4 + (int)$wsN * $wsR;
            if ($pWe + 4 > $len) {
                continue;
            }
            $u = unpack('V', substr($bin, $pWe, 4));
            $weN = $u ? ($u[1] & 0xffffffff) : 0;
            if ($weN < 1 || $weN > 500000) {
                continue;
            }
            foreach ($weCands as $weR) {
                $pAm = $pWe + 4 + (int)$weN * $weR;
                if ($pAm + 4 > $len) {
                    continue;
                }
                $u = unpack('V', substr($bin, $pAm, 4));
                $amN = $u ? ($u[1] & 0xffffffff) : 0;
                if ($amN < 1 || $amN > $amMax) {
                    continue;
                }
                $pAs = $pAm + 4 + (int)$amN * 68;
                $asOk = false;
                if ($pAs + 4 <= $len) {
                    $u = unpack('V', substr($bin, $pAs, 4));
                    $asN = $u ? ($u[1] & 0xffffffff) : 0;
                    $asOk = ($asN >= 1 && $asN <= 500000);
                }
                $score = 0;
                if ($wsR === 356) {
                    $score += 250;
                }
                if ($weR === 1424) {
                    $score += 250;
                }
                if ($asOk) {
                    $score += 180;
                }
                $score -= (int)(abs($wsR - 356) / 4);
                $score -= (int)(abs($weR - 1424) / 8);
                if ($score > $bestScore) {
                    $bestScore = $score;
                    $bestWs = $wsR;
                    $bestWe = $weR;
                }
            }
        }
        if ($bestWs === null || $bestWe === null) {
            return false;
        }
        $wsRec = (int)$bestWs;
        $weRec = (int)$bestWe;
        return true;
    }

    /**
     * Проверка цепочки после массива аддонов (для подбора шага аддона).
     *
     * @param int $wmBlockStart смещение u32 weapon_major count
     */
    private function weaponMajorSubEssenceCountsPlausible($bin, $wmBlockStart)
    {
        $ws = 0;
        $we = 0;
        if (!$this->tryResolveWeaponRows($bin, $wmBlockStart, $ws, $we)) {
            return false;
        }
        $this->weaponSubRecordSize = $ws;
        $this->weaponEssenceRecordSize = $we;
        return true;
    }

    /**
     * Порядок кандидатов шага EQUIPMENT_ADDON: сначала PWCATS_ADDON_STRIDE, затем 84 (vanilla pack4),
     * остальные по близости к 84. Раньше использовалась сортировка 64..320 — малые шаги давали ложные
     * совпадения по цепочке weapon_* и «съедали» таблицы (мусор в addons / items_count≈0).
     *
     * @return int[]
     */
    private function orderedAddonStrideCandidates()
    {
        $c = array();
        $env = getenv('PWCATS_ADDON_STRIDE');
        if ($env !== false && $env !== '' && ctype_digit((string)$env)) {
            $e = (int)$env;
            if ($e >= 32 && $e <= 512) {
                $c[] = $e;
            }
        }
        $rest = array();
        for ($s = 64; $s <= 320; $s += 4) {
            $rest[] = $s;
        }
        usort($rest, function ($a, $b) {
            $da = abs($a - 84);
            $db = abs($b - 84);
            if ($da !== $db) {
                return $da - $db;
            }
            return $a - $b;
        });
        foreach ($rest as $s) {
            if (!in_array($s, $c, true)) {
                $c[] = $s;
            }
        }
        return $c;
    }

    /**
     * Оценка правдоподобности разбора строк аддонов (id + UTF-16 name) для данного шага.
     *
     * @param int $dataStartByte
     * @param int $addonCount
     * @param int $stride
     * @return int чем выше, тем лучше
     */
    private function scoreAddonStridePlausibility($bin, $dataStartByte, $addonCount, $stride)
    {
        $len = strlen($bin);
        if ($stride < 68 || $dataStartByte + $stride > $len) {
            return 0;
        }
        $sample = min(48, $addonCount);
        $score = 0;
        for ($i = 0; $i < $sample; $i++) {
            $o = $dataStartByte + $i * $stride;
            if ($o + 68 > $len) {
                break;
            }
            $slice = substr($bin, $o, min($stride, 256));
            $br = new PwBinReader($slice);
            $id = $br->u32();
            $name = $br->utf16leZ(64);
            $pure = $id & 0x1fff;
            $nlen = strlen($name);
            $badCtrl = 0;
            for ($j = 0; $j < $nlen; $j++) {
                $c = ord($name[$j]);
                if ($c < 32 && $c !== 9 && $c !== 10 && $c !== 13) {
                    $badCtrl++;
                }
            }
            if ($nlen > 0 && $badCtrl > (int)($nlen * 0.25)) {
                continue;
            }
            if ($id > 0 && $id <= 0xffffff) {
                $score += 3;
            }
            if ($pure >= 1 && $pure <= 16000) {
                $score += 4;
            }
            if ($nlen >= 2) {
                $score += min(28, $nlen);
            } elseif ($nlen === 0 && $pure === 0 && $id === 0) {
                $score += 1;
            }
        }
        return $score;
    }

    /**
     * Подбор sizeof(EQUIPMENT_ADDON): на модах поля шире, чем в ванильном exptypes (84).
     *
     * @param int $dataStartByte offset первой записи аддона в $bin
     * @return int|null stride или null
     */
    private function detectEquipmentAddonStride($bin, $addonCount, $dataStartByte)
    {
        $len = strlen($bin);
        if ($addonCount <= 0 || $dataStartByte < 0) {
            return null;
        }
        $passing = array();
        foreach ($this->orderedAddonStrideCandidates() as $s) {
            $this->weaponSubRecordSize = 356;
            $this->weaponEssenceRecordSize = 1424;
            $end = $dataStartByte + $addonCount * $s;
            if ($end + 4 > $len) {
                continue;
            }
            $u = unpack('V', substr($bin, $end, 4));
            $wm = $u ? ($u[1] & 0xffffffff) : 0;
            if ($wm < 1 || $wm > 500000) {
                continue;
            }
            if ($this->weaponMajorSubEssenceCountsPlausible($bin, $end)) {
                $passing[$s] = $this->scoreAddonStridePlausibility($bin, $dataStartByte, $addonCount, $s);
            }
        }
        if ($passing === array()) {
            return null;
        }
        arsort($passing, SORT_NUMERIC);
        $bestStride = (int)key($passing);
        $bestScore = (int)reset($passing);
        $minTrust = 55;
        if ($bestScore < $minTrust && isset($passing[84])) {
            $bestStride = 84;
            $bestScore = (int)$passing[84];
            self::$lastLoadDiag .= '; addon_stride_favor_84_low_score';
        }
        self::$lastLoadDiag .= '; addon_stride_score=' . $bestScore;
        return $bestStride;
    }

    private function parseEquipmentAddons(PwBinReader $r, $fullBin = null)
    {
        $n = $r->u32();
        if ($n > 100000) {
            throw new InvalidArgumentException('addon count absurd');
        }
        $start = $r->tell();
        $sz = 84;
        $detected = null;
        if ($fullBin !== null && $n > 0) {
            $detected = $this->detectEquipmentAddonStride($fullBin, $n, $start);
            if ($detected !== null) {
                $sz = $detected;
                self::$lastLoadDiag .= '; addon_stride=' . $sz . ',weapon_sub_rec=' . $this->weaponSubRecordSize;
            }
        }
        if ($sz === 84) {
            $envSz = getenv('PWCATS_ADDON_STRIDE');
            if ($envSz !== false && $envSz !== '' && ctype_digit((string)$envSz)) {
                $sz = (int)$envSz;
            }
        }
        if ($sz < 32 || $sz > 512) {
            $sz = 84;
        }
        for ($i = 0; $i < $n; $i++) {
            $row = $r->raw($sz);
            $br = new PwBinReader($row);
            $id = $br->u32();
            $name = $br->utf16leZ(64);
            $pure = $id & 0x1fff;
            $this->mergeAddonEntry($pure, $name);
        }
    }

    private function indexWeaponEssences(PwBinReader $r)
    {
        $n = $r->u32();
        if ($n > 1000000) {
            throw new InvalidArgumentException('weapon essence count absurd');
        }
        $sz = $this->weaponEssenceRecordSize;
        if ($sz < 256 || $sz > 2048) {
            throw new InvalidArgumentException('weapon essence record size absurd: ' . $sz);
        }
        for ($i = 0; $i < $n; $i++) {
            $row = $r->raw($sz);
            $br = new PwBinReader($row);
            $id = $br->u32();
            $br->skip(8);
            $name = $br->utf16leZ(64);
            $this->mergeItemEntry($id, $name);
        }
    }

    private function indexArmorEssences(PwBinReader $r)
    {
        $n = $r->u32();
        if ($n > 1000000) {
            throw new InvalidArgumentException('armor essence count absurd');
        }
        $sz = $this->armorEssenceRecordSize;
        if ($sz < 800 || $sz > 1600) {
            throw new InvalidArgumentException('armor essence record size absurd: ' . $sz);
        }
        for ($i = 0; $i < $n; $i++) {
            $row = $r->raw($sz);
            $br = new PwBinReader($row);
            $id = $br->u32();
            $br->skip(8);
            $name = $br->utf16leZ(64);
            $this->mergeItemEntry($id, $name);
        }
    }

    private function indexDecorationEssences(PwBinReader $r)
    {
        $n = $r->u32();
        if ($n > 1000000) {
            throw new InvalidArgumentException('decoration essence count absurd');
        }
        $sz = $this->decorationEssenceRecordSize;
        if ($sz < 800 || $sz > 1600) {
            throw new InvalidArgumentException('decoration essence record size absurd: ' . $sz);
        }
        for ($i = 0; $i < $n; $i++) {
            $row = $r->raw($sz);
            $br = new PwBinReader($row);
            $id = $br->u32();
            $br->skip(8);
            $name = $br->utf16leZ(64);
            $this->mergeItemEntry($id, $name);
        }
    }

    private function indexStoneEssences(PwBinReader $r)
    {
        $n = $r->u32();
        if ($n > 1000000) {
            throw new InvalidArgumentException('stone essence count absurd');
        }
        $sz = $this->stoneEssenceRecordSize;
        if ($sz < 400 || $sz > 640) {
            throw new InvalidArgumentException('stone essence record size absurd: ' . $sz);
        }
        for ($i = 0; $i < $n; $i++) {
            $row = $r->raw($sz);
            $br = new PwBinReader($row);
            $id = $br->u32();
            $br->skip(4);
            $name = $br->utf16leZ(64);
            $this->mergeItemEntry($id, $name);
        }
    }
}
