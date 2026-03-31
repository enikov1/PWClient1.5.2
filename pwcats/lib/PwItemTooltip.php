<?php

require_once __DIR__ . '/PwItemSave.php';
require_once __DIR__ . '/PwElementsNames.php';

class PwItemTooltip
{
    private static $schoolNames = array('металл', 'дерево', 'вода', 'огонь', 'земля');

    const PROC_CASHITEM = 0x08;

    /** @var array|null id_pure => family (phys|magic|hp|defense|armor|res_all) */
    private static $refineIdFamily = null;

    /**
     * Соответствие INSERT_ADDON(*, refine_*) из gs/item/item_addon.cpp (ветка 1.5.x).
     * Величина бонуса — args[0] (уже с учётом уровня предмета и коэффициента заточки на GS).
     * Уровень «+N» — args[1] (см. equip_item::GetRefineLevel).
     */
    private static function refineIdFamilyMap()
    {
        if (self::$refineIdFamily !== null) {
            return self::$refineIdFamily;
        }
        $m = array();
        $add = function (array $ids, $family) use (&$m) {
            foreach ($ids as $id) {
                $m[(int)$id] = $family;
            }
        };
        $rg = function ($a, $b) {
            $o = array();
            for ($i = (int)$a; $i <= (int)$b; $i++) {
                $o[] = $i;
            }
            return $o;
        };
        $add($rg(1692, 1751), 'phys');
        $add(array(1497, 1498, 1499), 'phys');
        $add(array_merge(array(1496), $rg(1752, 1771)), 'magic');
        $add(array_merge($rg(1772, 1830), array(1500, 1501, 1502)), 'hp');
        $add(array_merge($rg(1832, 1851), array(1503)), 'defense');
        $add(array_merge($rg(1852, 1871), array(1504)), 'res_all');
        $add(array_merge($rg(1872, 1891), array(1505)), 'armor');
        self::$refineIdFamily = $m;
        return self::$refineIdFamily;
    }

    /** PWCATS_REFINE_ADDON_IDS: «123», «123:magic», «456:defense», … */
    private static function refineFamilyEnvOverrides()
    {
        static $cache = null;
        if ($cache !== null) {
            return $cache;
        }
        $cache = array();
        $env = getenv('PWCATS_REFINE_ADDON_IDS');
        if ($env === false || $env === '') {
            return $cache;
        }
        foreach (explode(',', $env) as $p) {
            $p = trim($p);
            if ($p === '') {
                continue;
            }
            if (preg_match('/^(\d+)\s*:\s*(phys|magic|hp|defense|armor|res_all)$/i', $p, $mm)) {
                $cache[(int)$mm[1]] = strtolower($mm[2]);
            } elseif (ctype_digit($p)) {
                $cache[(int)$p] = 'phys';
            }
        }
        return $cache;
    }

    private static function refineLevelFromAddonArgs(array $ad)
    {
        $args = isset($ad['args']) && is_array($ad['args']) ? $ad['args'] : array();
        if (count($args) >= 2) {
            $lv = (int)$args[1];
            if ($lv >= 1 && $lv <= 24) {
                return $lv;
            }
        }
        if (count($args) >= 1) {
            return max(0, (int)$args[0]);
        }
        return 0;
    }

    private static function looksLikeRefineAddon(array $ad, PwElementsNames $el)
    {
        $n = isset($ad['arg_count']) ? (int)$ad['arg_count'] : 0;
        $args = isset($ad['args']) && is_array($ad['args']) ? $ad['args'] : array();
        if ($n >= 2 && isset($args[1])) {
            $lv = (int)$args[1];
            if ($lv >= 1 && $lv <= 24 && (int)$args[0] >= 0) {
                return true;
            }
        }
        $pure = (int)$ad['id_pure'];
        $nm = $el->getAddonName($pure);
        if ($nm !== null && $nm !== '') {
            if (preg_match('/^W_[A-Z0-9_]+$/i', $nm)) {
                return true;
            }
            if (preg_match('/заточ|точн|кирк|укреп|refine/i', $nm)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @param string $essKind kind из essence: weapon|armor|decoration|...
     * @return string|null phys|magic|hp|defense|armor|res_all
     */
    private static function guessRefineFamily($pure, array $ad, PwElementsNames $el, $essKind)
    {
        $nm = $el->getAddonName($pure);
        if ($nm !== null && $nm !== '') {
            if (preg_match('/^W_[A-Z0-9_]+$/i', $nm)) {
                return 'phys';
            }
        }
        if ($essKind === 'weapon') {
            return 'phys';
        }
        if ($essKind === 'armor') {
            return 'defense';
        }
        if ($essKind === 'decoration') {
            return 'defense';
        }
        return 'phys';
    }

    /**
     * @param string $essKind
     * @return array{bonus:array,level:?int,skip_pures:array<int,bool>}
     */
    private static function analyzeRefines(array $addons, PwElementsNames $el, $essKind)
    {
        $bonus = array('phys' => 0, 'magic' => 0, 'defense' => 0, 'armor' => 0, 'hp' => 0, 'res_all' => 0);
        $maxLevel = 0;
        $skip = array();
        $byId = self::refineIdFamilyMap();
        $envOv = self::refineFamilyEnvOverrides();

        foreach ($addons as $ad) {
            $pure = (int)$ad['id_pure'];
            $family = null;
            if (isset($envOv[$pure])) {
                $family = $envOv[$pure];
            } elseif (isset($byId[$pure])) {
                $family = $byId[$pure];
            } elseif (self::looksLikeRefineAddon($ad, $el)) {
                $family = self::guessRefineFamily($pure, $ad, $el, $essKind);
            }
            if ($family === null) {
                continue;
            }
            $lv = self::refineLevelFromAddonArgs($ad);
            if ($lv > $maxLevel) {
                $maxLevel = $lv;
            }
            $val = isset($ad['args'][0]) ? (int)$ad['args'][0] : 0;
            if (isset($bonus[$family])) {
                $bonus[$family] += $val;
            }
            $skip[$pure] = true;
        }

        return array(
            'bonus' => $bonus,
            'level' => $maxLevel > 0 ? $maxLevel : null,
            'skip_pures' => $skip,
        );
    }

    /** Применяет сумму бонусов заточки к копии essence (база из сейва + args[0] аддона). */
    private static function applyRefineBonusesToEssence(array $ess, array $bonus)
    {
        $out = $ess;
        $k = isset($out['kind']) ? $out['kind'] : '';
        if ($k === 'weapon') {
            $out['damage_low'] = (int)$out['damage_low'] + (int)$bonus['phys'];
            $out['damage_high'] = (int)$out['damage_high'] + (int)$bonus['phys'];
            $out['magic_damage_low'] = (int)$out['magic_damage_low'] + (int)$bonus['magic'];
            $out['magic_damage_high'] = (int)$out['magic_damage_high'] + (int)$bonus['magic'];
        } elseif ($k === 'armor') {
            $out['defense'] = (int)$out['defense'] + (int)$bonus['defense'];
            $out['armor'] = (int)$out['armor'] + (int)$bonus['armor'];
            $out['hp_enhance'] = (int)$out['hp_enhance'] + (int)$bonus['hp'];
            if (!empty($out['resistance']) && is_array($out['resistance'])) {
                $nr = array();
                foreach ($out['resistance'] as $i => $v) {
                    $nr[$i] = (int)$v + (int)$bonus['res_all'];
                }
                $out['resistance'] = $nr;
            }
        } elseif ($k === 'decoration') {
            $out['damage'] = (int)$out['damage'] + (int)$bonus['phys'];
            $out['magic_damage'] = (int)$out['magic_damage'] + (int)$bonus['magic'];
            $out['defense'] = (int)$out['defense'] + (int)$bonus['defense'];
            $out['armor'] = (int)$out['armor'] + (int)$bonus['armor'];
            if (!empty($out['resistance']) && is_array($out['resistance'])) {
                $nr = array();
                foreach ($out['resistance'] as $i => $v) {
                    $nr[$i] = (int)$v + (int)$bonus['res_all'];
                }
                $out['resistance'] = $nr;
            }
        }
        return $out;
    }

    /**
     * @param array $detail
     * @param int|null $stallItemId
     * @param int|null $listingPrice цена в лавке (отобразится как на примере ComebackPW)
     */
    public static function fromDetail(array $detail, $stallItemId = null, $listingPrice = null)
    {
        if (empty($detail['item_save_hex'])) {
            return '';
        }
        $sizeT = 8;
        $env = getenv('PWCATS_SIZEOF_SIZET');
        if ($env === '4' || $env === 4) {
            $sizeT = 4;
        }
        $parser = new PwItemSave($sizeT);
        $parsed = $parser->parseHex($detail['item_save_hex']);
        if (is_string($parsed)) {
            return '<div class="item-decode-err mono" style="color:#ffa657;font-size:11px">' .
                htmlspecialchars($parsed, ENT_QUOTES, 'UTF-8') . '</div>';
        }
        $el = PwElementsNames::instance();
        return self::formatParsed($parsed, $el, $stallItemId, $detail, $listingPrice);
    }

    /**
     * Разбор item_save_hex в массив (для своего UI / decode=2 в listings.php).
     *
     * @param array $detail объект detail из GS (ключ item_save_hex)
     * @return array|null null если нет hex; иначе результат PwItemSave::parseHex (массив или с ключом error для строки)
     */
    public static function parseDetailToArray(array $detail)
    {
        if (empty($detail['item_save_hex'])) {
            return null;
        }
        $sizeT = 8;
        $env = getenv('PWCATS_SIZEOF_SIZET');
        if ($env === '4' || $env === 4) {
            $sizeT = 4;
        }
        $parser = new PwItemSave($sizeT);
        $parsed = $parser->parseHex($detail['item_save_hex']);
        if (is_string($parsed)) {
            return array('error' => $parsed);
        }
        return $parsed;
    }

    private static function h($s)
    {
        return htmlspecialchars((string)$s, ENT_QUOTES, 'UTF-8');
    }

    /** @return int bitmask; 0 = нет звезды */
    private static function starProcMask()
    {
        $m = getenv('PWCATS_STAR_PROC_MASK');
        if ($m !== false && $m !== '' && ctype_digit((string)$m)) {
            return (int)$m;
        }
        return self::PROC_CASHITEM;
    }

    private static function fmtPriceRu($n)
    {
        return number_format((int)$n, 0, ',', ' ');
    }

    private static function tipStat($label, $valueHtml, $valColor = '#3fb950')
    {
        return '<div class="pw-tip-row" style="font-size:13px;line-height:1.5;color:#e6edf3">' .
            self::h($label) .
            ' <span style="color:' . self::h($valColor) . ';font-weight:600">' . $valueHtml . '</span></div>';
    }

    /**
     * @param array $parsed
     * @param int|null $listingPrice
     */
    private static function formatParsed(
        array $parsed,
        PwElementsNames $el,
        $stallItemId,
        array $detail,
        $listingPrice
    ) {
        $tid = $stallItemId !== null ? (int)$stallItemId : 0;
        if ($tid <= 0) {
            $tid = isset($parsed['item_type']) ? (int)$parsed['item_type'] : 0;
        }
        $tplName = $el->getItemName($tid);
        $proc = isset($detail['proc_type']) ? (int)$detail['proc_type'] : 0;
        $mask = self::starProcMask();
        $starAlways = getenv('PWCATS_STAR_ALWAYS');
        $forceStar = ($starAlways === '1' || strcasecmp((string)$starAlways, 'true') === 0);
        $star = $forceStar || ($mask !== 0 && ($proc & $mask) !== 0)
            ? '<span style="color:#79c0ff">☆</span>'
            : '';

        $refineLevel = null;
        $skipRefinePures = array();
        $addonsExtra = array();
        $addonsEngraved = array();
        $essForLines = null;
        if (!empty($parsed['body']['addons']) && is_array($parsed['body']['addons'])
            && !empty($parsed['body']['essence']) && is_array($parsed['body']['essence'])) {
            $essKind = isset($parsed['body']['essence']['kind'])
                ? $parsed['body']['essence']['kind'] : '';
            $refineInfo = self::analyzeRefines($parsed['body']['addons'], $el, $essKind);
            $refineLevel = $refineInfo['level'];
            $skipRefinePures = $refineInfo['skip_pures'];
            $essForLines = self::applyRefineBonusesToEssence(
                $parsed['body']['essence'],
                $refineInfo['bonus']
            );
        }
        if (!empty($parsed['body']['addons']) && is_array($parsed['body']['addons'])) {
            foreach ($parsed['body']['addons'] as $ad) {
                $pid = (int)$ad['id_pure'];
                if (!empty($skipRefinePures[$pid])) {
                    continue;
                }
                if (!empty($ad['engraved'])) {
                    $addonsEngraved[] = $ad;
                } else {
                    $addonsExtra[] = $ad;
                }
            }
        }

        $titleName = ($tplName !== null && $tplName !== '') ? $tplName : ('Предмет #' . $tid);
        $refineSuffix = ($refineLevel !== null && $refineLevel > 0)
            ? ' <span style="color:#79c0ff;font-weight:600">+' . (int)$refineLevel . '</span>'
            : '';

        $blocks = array();
        $blocks[] =
            '<div class="pw-tip-title" style="font-size:15px;font-weight:600;color:#f0f6fc;line-height:1.35">' .
            $star . self::h($titleName) . $refineSuffix . '</div>';

        if ($listingPrice !== null && (int)$listingPrice > 0) {
            $blocks[] =
                '<div class="pw-tip-price" style="margin-top:6px;font-size:14px;color:#3fb950;font-weight:600">' .
                'Цена: ' . self::h(self::fmtPriceRu((int)$listingPrice)) . '</div>';
        }

        if (!empty($parsed['body']) && is_array($parsed['body'])) {
            $b = $parsed['body'];
            $blocks[] = '<hr style="border:none;border-top:1px solid #30363d;margin:10px 0" />';

            if (!empty($b['made']['crafter'])) {
                $blocks[] =
                    '<div class="pw-tip-creator" style="font-size:13px;line-height:1.5;color:#d2a8ff;margin-bottom:8px">' .
                    '<span style="color:#8b949e">Создатель:</span> <strong>' .
                    self::h($b['made']['crafter']) . '</strong></div>';
            }

            $statChunk = array();
            if ($essForLines !== null) {
                $statChunk = array_merge($statChunk, self::linesEssenceCard($essForLines));
            } elseif (!empty($b['essence']) && is_array($b['essence'])) {
                $statChunk = array_merge($statChunk, self::linesEssenceCard($b['essence']));
            }
            if (!empty($b['prereq']) && is_array($b['prereq'])) {
                $statChunk = array_merge($statChunk, self::linesPrereqCard($b['prereq']));
            }
            if (!empty($statChunk)) {
                $blocks[] = '<div class="pw-tip-stats">' . implode('', $statChunk) . '</div>';
            }

            if (!empty($b['sockets']) && is_array($b['sockets'])) {
                $sockLines = self::linesSockets($b['sockets'], $el);
                if (!empty($sockLines)) {
                    $blocks[] =
                        '<div style="margin-top:8px;padding-top:8px;border-top:1px solid #21262d">' .
                        implode('', $sockLines) . '</div>';
                }
            }
            if (!empty($addonsEngraved)) {
                $blocks[] =
                    '<div style="margin-top:8px;padding-top:8px;border-top:1px solid #21262d">' .
                    implode('', self::linesEngraving($addonsEngraved, $el)) . '</div>';
            }
            if (!empty($addonsExtra)) {
                $blocks[] =
                    '<div style="margin-top:8px;padding-top:8px;border-top:1px solid #21262d">' .
                    implode('', self::linesAddonsCompact($addonsExtra, $el)) . '</div>';
            }
        } else {
            $blocks[] =
                '<div class="mono" style="font-size:12px;color:#8b9cb3;margin-top:6px">' .
                'Нет данных экипировки в сейве.</div>';
        }

        $html = implode('', $blocks);
        if ($html === '') {
            return '';
        }
        return '<div class="pw-item-tooltip" style="padding:12px 14px;background:#161b22;border:1px solid #30363d;' .
            'border-radius:8px;min-width:260px;max-width:400px;box-sizing:border-box">' . $html . '</div>';
    }

    private static function linesPrereqCard(array $p)
    {
        $out = array();
        $out[] = self::tipStat('Прочность', self::h((int)$p['durability'] . '/' . (int)$p['max_durability']), '#3fb950');
        if ((int)$p['level'] > 0) {
            $out[] = self::tipStat('Требуемый уровень:', (string)(int)$p['level'], '#3fb950');
        }
        if ((int)$p['strength'] > 0) {
            $out[] = self::tipStat('Требуемая сила:', (string)(int)$p['strength'], '#3fb950');
        }
        if ((int)$p['agility'] > 0) {
            $out[] = self::tipStat('Требуемая ловкость:', (string)(int)$p['agility'], '#3fb950');
        }
        if ((int)$p['vitality'] > 0) {
            $out[] = self::tipStat('Требуемое телосложение:', (string)(int)$p['vitality'], '#3fb950');
        }
        if ((int)$p['energy'] > 0) {
            $out[] = self::tipStat('Требуемая энергия:', (string)(int)$p['energy'], '#3fb950');
        }
        return $out;
    }

    private static function linesEssenceCard(array $ess)
    {
        $out = array();
        $k = isset($ess['kind']) ? $ess['kind'] : '';
        if ($k === 'weapon') {
            if (isset($ess['weapon_level']) && (int)$ess['weapon_level'] > 0) {
                $out[] =
                    '<div class="pw-tip-row" style="font-size:12px;line-height:1.45;color:#8b949e">' .
                    'Уровень снаряжения ' . self::h((int)$ess['weapon_level']) . '</div>';
            }
            $out[] = self::tipStat(
                'Физ. урон',
                self::h((int)$ess['damage_low'] . '–' . (int)$ess['damage_high']),
                '#3fb950'
            );
            $out[] = self::tipStat(
                'Маг. урон',
                self::h((int)$ess['magic_damage_low'] . '–' . (int)$ess['magic_damage_high']),
                '#3fb950'
            );
            $out[] = self::tipStat('Дальность', self::h(sprintf('%.1f', (float)$ess['attack_range'])), '#8b949e');
        } elseif ($k === 'armor') {
            $out[] = self::tipStat('Защита', '+' . (int)$ess['defense'], '#3fb950');
            if ((int)$ess['hp_enhance'] > 0) {
                $out[] = self::tipStat('Здоровье', '+' . (int)$ess['hp_enhance'], '#3fb950');
            }
            if ((int)$ess['mp_enhance'] > 0) {
                $out[] = self::tipStat('Мана', '+' . (int)$ess['mp_enhance'], '#3fb950');
            }
            if ((int)$ess['armor'] !== 0) {
                $out[] = self::tipStat('Броня', '+' . (int)$ess['armor'], '#3fb950');
            }
            if (!empty($ess['resistance']) && is_array($ess['resistance'])) {
                foreach ($ess['resistance'] as $i => $v) {
                    if ((int)$v === 0) {
                        continue;
                    }
                    $sn = isset(self::$schoolNames[$i]) ? self::$schoolNames[$i] : ('стихия ' . $i);
                    $out[] = self::tipStat('Защита от ' . $sn, '+' . (int)$v, '#3fb950');
                }
            }
        } elseif ($k === 'decoration') {
            if ((int)$ess['damage'] !== 0 || (int)$ess['magic_damage'] !== 0) {
                $out[] = self::tipStat(
                    'Урон',
                    '+' . (int)$ess['damage'] . ' / +' . (int)$ess['magic_damage'],
                    '#3fb950'
                );
            }
            if ((int)$ess['defense'] !== 0 || (int)$ess['armor'] !== 0) {
                $out[] = self::tipStat(
                    'Защита',
                    '+' . (int)$ess['defense'] . ' / +' . (int)$ess['armor'],
                    '#3fb950'
                );
            }
            if (!empty($ess['resistance']) && is_array($ess['resistance'])) {
                foreach ($ess['resistance'] as $i => $v) {
                    if ((int)$v === 0) {
                        continue;
                    }
                    $sn = isset(self::$schoolNames[$i]) ? self::$schoolNames[$i] : ('стихия ' . $i);
                    $out[] = self::tipStat('Защита от ' . $sn, '+' . (int)$v, '#3fb950');
                }
            }
        } elseif ($k === 'projectile') {
            $out[] = self::tipStat(
                'Урон / масштаб',
                '+' . (int)$ess['enhance_damage'] . ' · ×' . (int)$ess['scale_enhance_damage'],
                '#3fb950'
            );
        } elseif ($k === 'unknown') {
            $out[] =
                '<div style="font-size:12px;color:#ffa657">Тип тела ' .
                (isset($ess['class_id']) ? (int)$ess['class_id'] : '?') .
                ' — детальный разбор отсутствует.</div>';
        }
        if (!empty($ess['warn'])) {
            $out[] = '<div style="font-size:11px;color:#ffa657;margin-top:4px">' . self::h($ess['warn']) . '</div>';
        }
        return $out;
    }

    private static function linesSockets(array $socks, PwElementsNames $el)
    {
        $ids = array();
        if (!empty($socks['stone_item_ids']) && is_array($socks['stone_item_ids'])) {
            $ids = $socks['stone_item_ids'];
        }
        $sockCount = isset($socks['count']) ? (int)$socks['count'] : count($ids);
        $filled = 0;
        foreach ($ids as $sid) {
            if ((int)$sid !== 0) {
                $filled++;
            }
        }
        if ($filled === 0) {
            if ($sockCount > 0) {
                return array(
                    '<div style="font-size:12px;color:#79c0ff;font-weight:600;margin-bottom:4px">Камни</div>' .
                    '<div style="font-size:12px;color:#8b949e">Гнёзд: ' . (int)$sockCount .
                    ', вплавленных нет</div>',
                );
            }
            return array();
        }
        $out = array(
            '<div style="font-size:12px;color:#79c0ff;font-weight:600;margin-bottom:4px">Вплавленные камни</div>',
        );
        foreach ($ids as $idx => $sid) {
            if ((int)$sid === 0) {
                continue;
            }
            $nm = $el->getItemName((int)$sid);
            $label = $nm !== null && $nm !== '' ? $nm : ('id ' . (int)$sid);
            $out[] = self::tipStat('Гнездо ' . ((int)$idx + 1), self::h($label), '#8b949e');
        }
        return $out;
    }

    private static function linesEngraving(array $addons, PwElementsNames $el)
    {
        if (empty($addons)) {
            return array();
        }
        $out = array(
            '<div style="font-size:12px;color:#d2a8ff;font-weight:600;margin-bottom:4px">Гравировка</div>',
        );
        foreach ($addons as $ad) {
            $pure = (int)$ad['id_pure'];
            $nm = $el->getAddonName($pure);
            $base = $nm !== null && $nm !== '' ? $nm : ('#' . $pure);
            $args = array();
            if (!empty($ad['args']) && is_array($ad['args'])) {
                foreach ($ad['args'] as $a) {
                    $args[] = (string)(int)$a;
                }
            }
            $argStr = $args ? (' (' . self::h(implode(', ', $args)) . ')') : '';
            $out[] =
                '<div style="font-size:12px;color:#e6edf3;line-height:1.45">» ' .
                self::h($base) . $argStr . '</div>';
        }
        return $out;
    }

    private static function linesAddonsCompact(array $addons, PwElementsNames $el)
    {
        if (empty($addons)) {
            return array();
        }
        $out = array(
            '<div style="font-size:12px;color:#79c0ff;font-weight:600;margin-bottom:4px">Доп. свойства</div>',
        );
        foreach ($addons as $ad) {
            $pure = (int)$ad['id_pure'];
            $nm = $el->getAddonName($pure);
            $base = $nm !== null && $nm !== '' ? $nm : ('#' . $pure);
            $args = array();
            foreach ($ad['args'] as $a) {
                $args[] = (string)(int)$a;
            }
            $argStr = $args ? (' (' . self::h(implode(', ', $args)) . ')') : '';
            $out[] =
                '<div style="font-size:12px;color:#8b949e;line-height:1.45">· ' .
                self::h($base) . $argStr . '</div>';
        }
        return $out;
    }
}
