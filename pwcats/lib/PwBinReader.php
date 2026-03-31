<?php

/**
 * Little-endian binary reader (gameserver archive / elements.data).
 */
class PwBinReader
{
    /** @var string */
    private $buf;
    /** @var int */
    private $off;
    /** @var int */
    private $len;

    public function __construct($binary)
    {
        $this->buf = $binary;
        $this->len = strlen($binary);
        $this->off = 0;
    }

    public function tell()
    {
        return $this->off;
    }

    /** @return int unsigned */
    public function peekU32()
    {
        if ($this->off + 4 > $this->len) {
            throw new InvalidArgumentException('PwBinReader: peekU32 past end');
        }
        $p = unpack('V', substr($this->buf, $this->off, 4));
        return $p ? ($p[1] & 0xffffffff) : 0;
    }

    /** @return int|null unsigned, null если выход за границу буфера */
    public function peekU32At($absOffset)
    {
        if ($absOffset < 0 || $absOffset + 4 > $this->len) {
            return null;
        }
        $p = unpack('V', substr($this->buf, $absOffset, 4));
        return $p ? ($p[1] & 0xffffffff) : null;
    }

    public function remaining()
    {
        return $this->len - $this->off;
    }

    public function eof()
    {
        return $this->off >= $this->len;
    }

    public function skip($n)
    {
        if ($this->off + $n > $this->len) {
            throw new InvalidArgumentException('PwBinReader: skip past end');
        }
        $this->off += $n;
    }

    public function raw($n)
    {
        if ($this->off + $n > $this->len) {
            throw new InvalidArgumentException('PwBinReader: raw past end');
        }
        $s = substr($this->buf, $this->off, $n);
        $this->off += $n;
        return $s;
    }

    public function u8()
    {
        return ord($this->raw(1));
    }

    public function i8()
    {
        $b = $this->u8();
        if ($b > 127) {
            $b -= 256;
        }
        return $b;
    }

    public function u16()
    {
        $s = $this->raw(2);
        $a = ord($s[0]) | (ord($s[1]) << 8);
        return $a & 0xffff;
    }

    public function i16()
    {
        $u = $this->u16();
        if ($u > 0x7fff) {
            return $u - 0x10000;
        }
        return $u;
    }

    public function u32()
    {
        $s = $this->raw(4);
        $a = ord($s[0]) | (ord($s[1]) << 8) | (ord($s[2]) << 16) | (ord($s[3]) << 24);
        return $a & 0xffffffff;
    }

    public function i32()
    {
        $u = $this->u32();
        if ($u > 0x7fffffff) {
            return $u - 0x100000000;
        }
        return $u;
    }

    public function u64()
    {
        $lo = $this->u32();
        $hi = $this->u32();
        return ($hi << 32) | $lo;
    }

    public function i64AsSize()
    {
        // size_t on LP64
        $lo = $this->u32();
        $hi = $this->u32();
        if ($hi !== 0 && $hi !== -1) {
            // 64-bit value; return as float string unsafe — use for sizes only when hi==0 for PW
        }
        return $lo;
    }

    public function f32()
    {
        $s = $this->raw(4);
        $a = unpack('g', $s);
        if ($a === false) {
            return 0.0;
        }
        return (float)$a[1];
    }

    public function utf16leZ($maxBytes)
    {
        $raw = $this->raw($maxBytes);
        $i = 0;
        while ($i + 1 < $maxBytes && ($raw[$i] !== "\x00" || $raw[$i + 1] !== "\x00")) {
            $i += 2;
        }
        $z = substr($raw, 0, $i);
        if ($z === '') {
            return '';
        }
        if (function_exists('mb_convert_encoding')) {
            return mb_convert_encoding($z, 'UTF-8', 'UTF-16LE');
        }
        if (function_exists('iconv')) {
            $s = @iconv('UTF-16LE', 'UTF-8//IGNORE', $z);
            return $s !== false ? $s : '';
        }
        return $z;
    }
}
