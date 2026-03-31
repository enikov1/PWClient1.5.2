<?php
/**
 * POST application/json — тот же контракт, что у gs / gdeliveryd (HTTP ingest).
 * Настройка игры: LISTING_SYNC_HOST, LISTING_SYNC_PORT, LISTING_SYNC_PATH=/pwcats/ingest.php
 */

header('Content-Type: application/json; charset=utf-8');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    header('Access-Control-Allow-Origin: *');
    header('Access-Control-Allow-Methods: POST, OPTIONS');
    header('Access-Control-Allow-Headers: Content-Type, X-Listing-Sync-Secret, X-Stall-Sync-Secret');
    http_response_code(204);
    exit;
}

header('Access-Control-Allow-Origin: *');

require __DIR__ . '/store.php';

$secret = getenv('LISTING_SYNC_SECRET');
if ($secret === false || $secret === '') {
    $secret = '';
}

if ($secret !== '') {
    $got = '';
    if (isset($_SERVER['HTTP_X_LISTING_SYNC_SECRET'])) {
        $got = (string)$_SERVER['HTTP_X_LISTING_SYNC_SECRET'];
    } elseif (isset($_SERVER['HTTP_X_STALL_SYNC_SECRET'])) {
        // gameserver cgame/gs/stall_website_sync.cpp
        $got = (string)$_SERVER['HTTP_X_STALL_SYNC_SECRET'];
    }
    if ($got !== $secret) {
        http_response_code(401);
        echo json_encode(array(
            'ok' => false,
            'error' => 'bad or missing secret header (X-Listing-Sync-Secret or X-Stall-Sync-Secret)',
        ));
        exit;
    }
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(array('ok' => false, 'error' => 'POST only'));
    exit;
}

$raw = file_get_contents('php://input');
$body = json_decode($raw, true);
if (!is_array($body)) {
    http_response_code(400);
    echo json_encode(array('ok' => false, 'error' => 'expect application/json object'));
    exit;
}

$body = pwcats_normalize_listing_body($body);

$id = pwcats_next_id();
$entry = array(
    'id' => $id,
    'at' => gmdate('c'),
    'ip' => isset($_SERVER['REMOTE_ADDR']) ? (string)$_SERVER['REMOTE_ADDR'] : '',
    'body' => $body,
);
pwcats_listings_push($entry);

echo json_encode(array('ok' => true, 'id' => $id));
