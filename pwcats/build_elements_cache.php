<?php
/**
 * CLI: строит data/elements_names.json из клиентского/серверного elements.data
 * (имена аддонов, оружия, брони, бижи, камней — для тултипов лавки).
 *
 * Запуск из каталога pwcats на сервере:
 *   cd /var/www/html/pwcats && php build_elements_cache.php
 *
 * Если по привычке пишете префикс pwcats/ уже из этого каталога:
 *   php pwcats/build_elements_cache.php
 * (это отдельная обёртка pwcats/pwcats/build_elements_cache.php)
 *
 * Или с явным путём:
 *   php build_elements_cache.php /path/to/elements.data
 *
 * По умолчанию читает PWCATS_ELEMENTS_DATA или /root/pwserver/gamed/config/elements.data.
 * Выход: PWCATS_ELEMENTS_JSON или data/elements_names.json в этом каталоге.
 *
 * Размеры структур подбираются; при сбое см. lib/PwElementsNames.php: PWCATS_WEAPON_ESS_REC,
 * PWCATS_ARMOR_ESS_REC, PWCATS_DECO_SUB_REC, PWCATS_DECORATION_ESS_REC, PWCATS_MED_SUB_REC,
 * PWCATS_MED_ESS_REC (байты на запись).
 * Схема PW_*.cfg (ElementEditor): положите рядом PW_1.5.3_v145.cfg или задайте PWCATS_ELEMENTS_CFG —
 * тогда sizeof берутся из cfg и цепочка weapon→armor→… не перебирается.
 */

if (PHP_SAPI !== 'cli') {
    header('HTTP/1.0 403 Forbidden');
    echo 'CLI only';
    exit(1);
}

if ($argc >= 2 && $argv[1] !== '') {
    $src = $argv[1];
} else {
    $src = getenv('PWCATS_ELEMENTS_DATA');
    if ($src === false || $src === '') {
        $src = '/root/pwserver/gamed/config/elements.data';
    }
    if (!is_file($src)) {
        $src = __DIR__ . '/data/elements.data';
    }
}

if (!is_file($src)) {
    fwrite(STDERR, "Файл elements.data не найден: ни аргумент, ни PWCATS_ELEMENTS_DATA, ни /root/pwserver/...\n\n");
    fwrite(STDERR, "Уже находитесь в каталоге pwcats — запускайте:\n");
    fwrite(STDERR, "  php build_elements_cache.php\n");
    fwrite(STDERR, "или с путём:\n");
    fwrite(STDERR, "  php build_elements_cache.php /root/pwserver/gamed/config/elements.data\n");
    fwrite(STDERR, "\n(Не используйте префикс pwcats/ в пути к скрипту, если cwd уже /var/www/html/pwcats.)\n");
    exit(1);
}

require_once __DIR__ . '/lib/PwElementsNames.php';

fwrite(STDOUT, "Source: {$src}\n");

$out = getenv('PWCATS_ELEMENTS_JSON');
if ($out === false || $out === '') {
    $out = __DIR__ . '/data/elements_names.json';
}

$dir = dirname($out);
if (!is_dir($dir)) {
    mkdir($dir, 0755, true);
}

$man = new PwElementsNames();
$man->load('', $src);

$payload = array(
    'cache_version' => 2,
    'expected_elements_magic' => '0x30000091',
    'addons_count' => count($man->getAddonsMap()),
    'items_count' => count($man->getItemsMap()),
    'addons' => $man->getAddonsMap(),
    'items' => $man->getItemsMap(),
);

file_put_contents(
    $out,
    json_encode($payload, JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT),
    LOCK_EX
);

fwrite(STDOUT, "Wrote {$out} (" . count($man->getAddonsMap()) . " addons, " .
    count($man->getItemsMap()) . " items)\n");

if (count($man->getAddonsMap()) === 0) {
    fwrite(STDERR, "Warning: 0 addons — см. диагностику ниже.\n");
    fwrite(STDERR, PwElementsNames::getLastLoadDiag() . "\n");
    fwrite(STDERR, "Подсказки: другой ELEMENTDATA_VERSION / порядок таблиц / размеры struct в exptypes.h; для time_t после exporter block: PWCATS_EXPORTER_TIMET_SKIP=4.\n");
}
