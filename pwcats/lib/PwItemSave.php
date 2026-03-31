<?php

require_once __DIR__ . '/PwBinReader.php';

/**
 * Parses gameserver item::Save(raw_wrapper) stream (see cgame/gs/item.h).
 *
 * После guid: два байта bool (IsActive, IsSecActive), сразу int sec_param, int classid, size_t len, тело.
 * В этом дереве archive пишет bool как char — без padding перед int.
 *
 * Env: PWCATS_ITEM_BODY_BOOL_PAD=1 — пропустить 2 байта после двух bool (некоторые сборки/клиенты).
 *      PWCATS_ITEM_BODY_BOOL_PAD=0 — явно без pad. Не задано — авто: перебор pad и при необходимости size_t 8/4.
 *      PWCATS_SIZEOF_SIZET=4|8 — размер count/pile_limit/price/len (по умолчанию пробуются оба при авто).
 */
class PwItemSave
{
    const CLS_WEAPON = 402;
    const CLS_RANGE_WEAPON = 403;
    const CLS_MELEE_WEAPON = 404;
    const CLS_ARMOR = 405;
    const CLS_PROJECTILE = 406;
    const CLS_DECORATION = 407;

    /** @var int 4 or 8 */
    private $sizeT;

    public function __construct($sizeTBytes = 8)
    {
        $this->sizeT = ($sizeTBytes === 4) ? 4 : 8;
    }

    /**
     * @param string $hex
     * @return array|string error starting with "error:"
     */
    public function parseHex($hex)
    {
        $hex = preg_replace('/\s+/', '', $hex);
        if (strlen($hex) % 2 !== 0) {
            return 'error: odd hex length';
        }
        $bin = hex2bin($hex);
        if ($bin === false) {
            return 'error: invalid hex';
        }
        try {
            return $this->parseBinary($bin);
        } catch (Exception $e) {
            return 'error: ' . $e->getMessage();
        }
    }

    private function readSizeT(PwBinReader $r)
    {
        if ($this->sizeT === 8) {
            $lo = $r->u32();
            $hi = $r->u32();
            if ($hi !== 0) {
                // keep as string for display
                return sprintf('%u', $lo); // lossy if huge
            }
            return $lo;
        }
        return $r->u32();
    }

    public function parseBinary($bin)
    {
        $stEnv = getenv('PWCATS_SIZEOF_SIZET');
        $forcedSt = null;
        if ($stEnv === '4' || $stEnv === 4) {
            $forcedSt = 4;
        } elseif ($stEnv === '8' || $stEnv === 8) {
            $forcedSt = 8;
        }

        $padEnv = getenv('PWCATS_ITEM_BODY_BOOL_PAD');
        $forcedPad = null;
        if ($padEnv === '0' || $padEnv === 0) {
            $forcedPad = false;
        } elseif ($padEnv === '1' || $padEnv === 1) {
            $forcedPad = true;
        }

        if ($forcedSt !== null && $forcedPad !== null) {
            $this->sizeT = $forcedSt;
            return $this->parseBinaryCore($bin, $forcedPad);
        }

        $stVariants = $forcedSt !== null ? array($forcedSt) : array(8, 4);
        $padVariants = $forcedPad !== null ? array($forcedPad) : array(false, true);

        $last = null;
        foreach ($stVariants as $st) {
            foreach ($padVariants as $pad) {
                try {
                    $this->sizeT = $st;
                    return $this->parseBinaryCore($bin, $pad);
                } catch (InvalidArgumentException $e) {
                    $last = $e;
                    $m = $e->getMessage();
                    if (strpos($m, 'body truncated') === false
                        && stripos($m, 'past end') === false
                    ) {
                        throw $e;
                    }
                }
            }
        }
        if ($last) {
            throw $last;
        }
        throw new InvalidArgumentException('body truncated');
    }

    /**
     * @param bool $boolPad пропустить 2 байта после пары bool перед int sec_param
     * @return array
     */
    private function parseBinaryCore($bin, $boolPad)
    {
        $r = new PwBinReader($bin);
        $out = array();
        $type = $r->i32();
        $out['item_type'] = $type;
        if ($type === -1) {
            $out['note'] = 'пустой слот';
            return $out;
        }
        $out['count'] = $this->readSizeT($r);
        $out['pile_limit'] = $this->readSizeT($r);
        $out['equip_mask'] = $r->i32();
        $out['proc_type'] = $r->u32();
        $out['price'] = $this->readSizeT($r);
        $out['expire_date'] = $r->i32();
        $out['guid1'] = $r->i32();
        $out['guid2'] = $r->i32();

        $active = $r->u8();
        $secActive = $r->u8();
        $out['body_active'] = $active !== 0;
        $out['body_sec_active'] = $secActive !== 0;
        if ($boolPad) {
            $r->skip(2);
        }
        $out['body_sec_param'] = $r->i32();
        $classId = $r->i32();
        $out['body_class_id'] = $classId;
        if ($classId <= 0) {
            $out['body'] = null;
            return $out;
        }
        $len = $this->readSizeT($r);
        $out['body_blob_len'] = $len;
        if ($len <= 0) {
            $out['body'] = null;
            return $out;
        }
        if ($r->remaining() < $len) {
            throw new InvalidArgumentException('body truncated');
        }
        $bodyBin = $r->raw((int)$len);
        $out['body'] = $this->parseEquipBody(new PwBinReader($bodyBin), $classId);
        return $out;
    }

    /**
     * _raw_data stream: SaveLimit, SaveEssence, SaveSocketData, SaveAddOn
     *
     * @return array
     */
    private function parseEquipBody(PwBinReader $r, $classId)
    {
        $b = array('class_id' => $classId);
        $b['prereq'] = $this->parsePrereq($r);
        $b['made'] = $this->parseMadeTag($r);
        $b['essence'] = $this->parseEssence($r, $classId, $b['made']['ess_decl_size']);
        $b['sockets'] = $this->parseSockets($r);
        $b['addons'] = $this->parseAddons($r);
        if (!$r->eof() && $r->remaining() > 0) {
            $b['tail_unread'] = $r->remaining();
        }
        return $b;
    }

    private function parsePrereq(PwBinReader $r)
    {
        return array(
            'level' => $r->i16(),
            'race' => $r->i16(),
            'strength' => $r->i16(),
            'vitality' => $r->i16(),
            'agility' => $r->i16(),
            'energy' => $r->i16(),
            'durability' => $r->i32(),
            'max_durability' => $r->i32(),
        );
    }

    private function parseMadeTag(PwBinReader $r)
    {
        $essSize = $r->u16();
        $tagType = $r->i8();
        $tagSize = $r->u8();
        $content = '';
        if ($tagSize > 0) {
            $content = $r->utf16leZ($tagSize);
        }
        return array(
            'ess_decl_size' => $essSize,
            'tag_type' => $tagType,
            'tag_size' => $tagSize,
            'crafter' => $content,
        );
    }

    private function parseEssence(PwBinReader $r, $classId, $declSize)
    {
        switch ($classId) {
            case self::CLS_WEAPON:
            case self::CLS_RANGE_WEAPON:
            case self::CLS_MELEE_WEAPON:
                if ($declSize !== 44) {
                    $n = min((int)$declSize, $r->remaining());
                    return array(
                        'kind' => 'weapon',
                        'warn' => 'unexpected weapon essence size ' . $declSize,
                        'raw_hex' => $n > 0 ? bin2hex($r->raw($n)) : '',
                    );
                }
                return array(
                    'kind' => 'weapon',
                    'weapon_type' => $r->i16(),
                    'weapon_delay' => $r->i16(),
                    'weapon_class' => $r->i32(),
                    'weapon_level' => $r->i32(),
                    'require_projectile' => $r->i32(),
                    'damage_low' => $r->i32(),
                    'damage_high' => $r->i32(),
                    'magic_damage_low' => $r->i32(),
                    'magic_damage_high' => $r->i32(),
                    'attack_speed' => $r->i32(),
                    'attack_range' => $r->f32(),
                    'attack_short_range' => $r->f32(),
                );
            case self::CLS_ARMOR:
                if ($declSize !== 36) {
                    $n = min((int)$declSize, $r->remaining());
                    return array(
                        'kind' => 'armor',
                        'warn' => 'unexpected armor essence size ' . $declSize,
                        'raw_hex' => $n > 0 ? bin2hex($r->raw($n)) : '',
                    );
                }
                $ess = array(
                    'kind' => 'armor',
                    'defense' => $r->i32(),
                    'armor' => $r->i32(),
                    'mp_enhance' => $r->i32(),
                    'hp_enhance' => $r->i32(),
                    'resistance' => array(),
                );
                for ($i = 0; $i < 5; $i++) {
                    $ess['resistance'][$i] = $r->i32();
                }
                return $ess;
            case self::CLS_DECORATION:
                if ($declSize !== 36) {
                    $n = min((int)$declSize, $r->remaining());
                    return array(
                        'kind' => 'decoration',
                        'warn' => 'unexpected decoration essence size ' . $declSize,
                        'raw_hex' => $n > 0 ? bin2hex($r->raw($n)) : '',
                    );
                }
                $ess = array(
                    'kind' => 'decoration',
                    'damage' => $r->i32(),
                    'magic_damage' => $r->i32(),
                    'defense' => $r->i32(),
                    'armor' => $r->i32(),
                    'resistance' => array(),
                );
                for ($i = 0; $i < 5; $i++) {
                    $ess['resistance'][$i] = $r->i32();
                }
                return $ess;
            case self::CLS_PROJECTILE:
                if ($declSize !== 20) {
                    $n = min((int)$declSize, $r->remaining());
                    return array(
                        'kind' => 'projectile',
                        'warn' => 'unexpected projectile essence size ' . $declSize,
                        'raw_hex' => $n > 0 ? bin2hex($r->raw($n)) : '',
                    );
                }
                return array(
                    'kind' => 'projectile',
                    'projectile_type' => $r->i32(),
                    'enhance_damage' => $r->i32(),
                    'scale_enhance_damage' => $r->i32(),
                    'weapon_level_low' => $r->i32(),
                    'weapon_level_high' => $r->i32(),
                );
            default:
                $skip = min((int)$declSize, $r->remaining());
                return array(
                    'kind' => 'unknown',
                    'class_id' => $classId,
                    'decl_size' => $declSize,
                    'skipped_hex' => $skip > 0 ? bin2hex($r->raw($skip)) : '',
                );
        }
    }

    private function parseSockets(PwBinReader $r)
    {
        $count = $r->u16();
        $mask = $r->u16();
        $types = array();
        for ($i = 0; $i < $count; $i++) {
            $types[] = $r->i32();
        }
        return array('count' => $count, 'modify_mask' => $mask, 'stone_item_ids' => $types);
    }

    private function parseAddons(PwBinReader $r)
    {
        $n = $this->readSizeT($r);
        $list = array();
        for ($i = 0; $i < $n; $i++) {
            $id = $r->i32();
            $argN = ($id & 0x6000) >> 13;
            $args = array();
            for ($j = 0; $j < $argN; $j++) {
                $args[] = $r->i32();
            }
            $list[] = array(
                'id_raw' => $id,
                'id_pure' => $id & 0x1fff,
                'arg_count' => $argN,
                'args' => $args,
                'embedded' => ($id & 0x8000) !== 0,
                'engraved' => ($id & 0x20000) !== 0,
                'inherited' => ($id & 0x40000) !== 0,
            );
        }
        return $list;
    }
}
