<?php
/**
 * POST — очистить сохранённые события.
 */

header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');

require __DIR__ . '/store.php';

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(array('ok' => false, 'error' => 'POST only'));
    exit;
}

pwcats_listings_clear();
echo json_encode(array('ok' => true));
