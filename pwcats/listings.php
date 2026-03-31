<?php
/**
 * GET ?source=cat|commission|auction (опционально) — JSON для AJAX.
 * GET decode=0 — не строить tooltip (быстрее).
 * GET decode=2 — как 1, плюс detail.tooltip_json: разбор item_save_hex (массив для своего UI).
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');

require __DIR__ . '/store.php';

$decodeParam = isset($_GET['decode']) ? (string)$_GET['decode'] : '';
$decodeItems = ($decodeParam !== '0');
$decodeJson = ($decodeParam === '2');

$source = isset($_GET['source']) ? strtolower((string)$_GET['source']) : '';
$list = pwcats_listings_read();

if ($source === 'cat' || $source === 'commission' || $source === 'auction') {
    $list = array_values(array_filter($list, function ($e) use ($source) {
        return isset($e['body']['listing_source']) && $e['body']['listing_source'] === $source;
    }));
}

$limit = isset($_GET['limit']) ? (int)$_GET['limit'] : 200;
if ($limit < 1) {
    $limit = 1;
}
if ($limit > 500) {
    $limit = 500;
}

$slice = array_slice($list, 0, $limit);

if ($decodeItems) {
    require_once __DIR__ . '/lib/PwItemTooltip.php';
    foreach ($slice as &$entry) {
        if (!isset($entry['body']['goods']) || !is_array($entry['body']['goods'])) {
            continue;
        }
        foreach ($entry['body']['goods'] as &$g) {
            if (!isset($g['detail']) || !is_array($g['detail'])) {
                continue;
            }
            if (empty($g['detail']['item_save_hex'])) {
                continue;
            }
            $iid = isset($g['item_id']) ? $g['item_id'] : null;
            $price = null;
            if (isset($g['price'])) {
                $price = $g['price'];
            } elseif (isset($g['sale_price'])) {
                $price = $g['sale_price'];
            } elseif (isset($g['purchase_price'])) {
                $price = $g['purchase_price'];
            }
            $g['detail']['tooltip_html'] = PwItemTooltip::fromDetail($g['detail'], $iid, $price);
            if ($decodeJson) {
                $g['detail']['tooltip_json'] = PwItemTooltip::parseDetailToArray($g['detail']);
            }
        }
        unset($g);
    }
    unset($entry);
}

echo json_encode(array(
    'ok' => true,
    'count' => count($list),
    'items' => $slice,
), JSON_UNESCAPED_UNICODE);
