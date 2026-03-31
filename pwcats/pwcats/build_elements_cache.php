<?php
/**
 * Обёртка для привычной команды из cwd=/var/www/html/pwcats:
 *   php pwcats/build_elements_cache.php
 * Делегирует родительскому build_elements_cache.php (тот же $argc/$argv).
 */
require __DIR__ . '/../build_elements_cache.php';
