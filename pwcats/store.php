<?php
/**
 * File storage for listing ingest (PHP 7.4+).
 */

/**
 * Приводит тело от gameserver (stall_website_sync) к полям, которые ждёт UI (index.php).
 */
function pwcats_normalize_listing_body(array $b)
{
    if (!isset($b['listing_source']) || $b['listing_source'] !== 'cat') {
        return $b;
    }
    if (isset($b['event']) && !isset($b['action'])) {
        $b['action'] = $b['event'];
    }
    if (isset($b['roleid']) && !isset($b['role_id'])) {
        $b['role_id'] = $b['roleid'];
    }
    if (isset($b['trade_id']) && !isset($b['stall_trade_id'])) {
        $b['stall_trade_id'] = $b['trade_id'];
    }
    if (isset($b['pos']) && is_array($b['pos'])) {
        $p = $b['pos'];
        if (!isset($b['pos_x']) && array_key_exists('x', $p)) {
            $b['pos_x'] = $p['x'];
        }
        if (!isset($b['pos_y']) && array_key_exists('y', $p)) {
            $b['pos_y'] = $p['y'];
        }
        if (!isset($b['pos_z']) && array_key_exists('z', $p)) {
            $b['pos_z'] = $p['z'];
        }
    }
    if (isset($b['stall_name_hex']) && !isset($b['shop_name_hex'])) {
        $b['shop_name_hex'] = $b['stall_name_hex'];
    }
    if (isset($b['goods']) && is_array($b['goods'])) {
        foreach ($b['goods'] as $i => $g) {
            if (!is_array($g)) {
                continue;
            }
            if (!isset($g['side']) && isset($g['is_buy_order'])) {
                $b['goods'][$i]['side'] = ((int)$g['is_buy_order'] === 1) ? 'buy' : 'sell';
            }
        }
    }
    return $b;
}

function pwcats_data_dir()
{
    return __DIR__ . '/data';
}

function pwcats_listings_path()
{
    return pwcats_data_dir() . '/listings.json';
}

function pwcats_counter_path()
{
    return pwcats_data_dir() . '/counter.txt';
}

function pwcats_listings_read()
{
    $p = pwcats_listings_path();
    if (!is_file($p)) {
        return array();
    }
    $j = file_get_contents($p);
    $d = json_decode($j, true);
    return is_array($d) ? $d : array();
}

function pwcats_listings_write(array $a)
{
    $dir = pwcats_data_dir();
    if (!is_dir($dir)) {
        mkdir($dir, 0755, true);
    }
    $p = pwcats_listings_path();
    file_put_contents(
        $p,
        json_encode($a, JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT),
        LOCK_EX
    );
}

function pwcats_next_id()
{
    $dir = pwcats_data_dir();
    if (!is_dir($dir)) {
        mkdir($dir, 0755, true);
    }
    $f = pwcats_counter_path();
    $fp = fopen($f, 'c+');
    if ($fp === false) {
        return (int)(microtime(true) * 1000);
    }
    if (!flock($fp, LOCK_EX)) {
        fclose($fp);
        return (int)(microtime(true) * 1000);
    }
    $cur = (int)stream_get_contents($fp);
    $cur++;
    ftruncate($fp, 0);
    rewind($fp);
    fwrite($fp, (string)$cur);
    fflush($fp);
    flock($fp, LOCK_UN);
    fclose($fp);
    return $cur;
}

function pwcats_listings_push(array $entry)
{
    $max = 2000;
    $a = pwcats_listings_read();
    array_unshift($a, $entry);
    if (count($a) > $max) {
        $a = array_slice($a, 0, $max);
    }
    pwcats_listings_write($a);
}

function pwcats_listings_clear()
{
    pwcats_listings_write(array());
}
