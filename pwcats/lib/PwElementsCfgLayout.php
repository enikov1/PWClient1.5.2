<?php

/**
 * Разбор экспортной схемы вида PW_*.cfg (список полей + типы string:N / wstring:N / int32).
 * Сумма размеров полей совпадает с sizeof(struct) при #pragma pack(4) на той же модели, что и в cfg
 * (wstring — фиксированный буфер в байтах, как в файле).
 */
final class PwElementsCfgLayout
{
    /** @var array<string,int|null> имя блока => размер записи; RAW — null */
    private $sizeByName;

    /**
     * @param string $path
     * @throws InvalidArgumentException
     */
    public function __construct($path)
    {
        $this->sizeByName = self::parseFile($path);
    }

    /** @return array<string,int|null> */
    public function getRecordSizesByName()
    {
        return $this->sizeByName;
    }

    /**
     * @param string $name например WEAPON_ESSENCE
     * @return int|null null если нет в cfg или RAW
     */
    public function recordSize($name)
    {
        $name = (string)$name;
        if (!isset($this->sizeByName[$name])) {
            return null;
        }
        $s = $this->sizeByName[$name];
        return is_int($s) ? $s : null;
    }

    /**
     * @param string $path
     * @return array<string,int|null>
     */
    private static function parseFile($path)
    {
        if (!is_file($path) || !is_readable($path)) {
            throw new InvalidArgumentException('elements cfg not readable: ' . $path);
        }
        $lines = file($path, FILE_IGNORE_NEW_LINES);
        if ($lines === false) {
            throw new InvalidArgumentException('elements cfg read failed: ' . $path);
        }
        $n = count($lines);
        $i = 0;
        while ($i < $n && !preg_match('/^\d{3}\s+-\s+/u', trim($lines[$i]))) {
            $i++;
        }
        $out = array();
        while ($i < $n) {
            $header = trim($lines[$i]);
            if (!preg_match('/^\d{3}\s+-\s+(.+)$/u', $header, $m)) {
                $i++;
                continue;
            }
            $blockName = trim($m[1]);
            $i++;
            if ($i >= $n) {
                break;
            }
            $modeOrRaw = trim($lines[$i]);
            $i++;
            if ($modeOrRaw === 'RAW') {
                if ($i < $n) {
                    $i++;
                }
                $out[$blockName] = null;
                continue;
            }
            if ($i >= $n) {
                break;
            }
            // После 0 / 4 / AUTO идёт строка имён полей, затем строка типов.
            $i++;
            if ($i >= $n) {
                break;
            }
            $typesLine = trim($lines[$i]);
            $i++;
            $sz = self::typesLineToRecordSize($typesLine);
            $out[$blockName] = $sz;
        }
        return $out;
    }

    /**
     * @param string $typesLine
     * @return int|null
     */
    private static function typesLineToRecordSize($typesLine)
    {
        if ($typesLine === '' || stripos($typesLine, 'RAW') !== false) {
            return null;
        }
        $sum = 0;
        foreach (explode(';', $typesLine) as $tok) {
            $tok = trim($tok);
            if ($tok === '') {
                continue;
            }
            if ($tok === 'int32' || $tok === 'uint32' || $tok === 'float') {
                $sum += 4;
                continue;
            }
            if ($tok === 'int64' || $tok === 'uint64') {
                $sum += 8;
                continue;
            }
            if ($tok === 'int16' || $tok === 'uint16') {
                $sum += 2;
                continue;
            }
            if ($tok === 'int8' || $tok === 'uint8' || $tok === 'byte') {
                $sum += 1;
                continue;
            }
            if (preg_match('/^string:(\d+)$/i', $tok, $m)) {
                $sum += (int)$m[1];
                continue;
            }
            if (preg_match('/^wstring:(\d+)$/i', $tok, $m)) {
                $sum += (int)$m[1];
                continue;
            }
            if (preg_match('/^byte:(\d+)$/i', $tok, $m)) {
                $sum += (int)$m[1];
                continue;
            }
            if (strcasecmp($tok, 'byte:AUTO') === 0) {
                return null;
            }
            throw new InvalidArgumentException('PwElementsCfgLayout: unknown type token "' . $tok . '"');
        }
        return $sum;
    }

    /** Таблицы после exporter-блока в elementdataman (совпадает с нумерацией 021–036 в типичном cfg). */
    public static function tailTableNamesAfterExporter()
    {
        return array(
            'SKILLTOME_SUB_TYPE',
            'SKILLTOME_ESSENCE',
            'FLYSWORD_ESSENCE',
            'WINGMANWING_ESSENCE',
            'TOWNSCROLL_ESSENCE',
            'UNIONSCROLL_ESSENCE',
            'REVIVESCROLL_ESSENCE',
            'ELEMENT_ESSENCE',
            'TASKMATTER_ESSENCE',
            'TOSSMATTER_ESSENCE',
            'PROJECTILE_TYPE',
            'PROJECTILE_ESSENCE',
            'QUIVER_SUB_TYPE',
            'QUIVER_ESSENCE',
            'STONE_SUB_TYPE',
        );
    }
}
