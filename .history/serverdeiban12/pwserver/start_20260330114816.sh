#!/bin/bash

# --- Синхронизация лавок на сайт (pwcats/ingest.php; см. getenv в gs / gdeliveryd) ---
# У вас каталог на сервере: /var/www/html/pwcats → в URL это /pwcats/... (DocumentRoot = /var/www/html).
# Должно выполняться до запуска gdeliveryd и gs. Переопределение: LISTING_SYNC_HOST=1.2.3.4 ./start.sh
# На стороне сайта: тот же LISTING_SYNC_SECRET в php-fpm (env[...]) или Apache SetEnv для ingest.php.
# HTTPS с gs напрямую не поддержан — слушайте :80 или прокси на HTTP внутри.
LISTING_SYNC_HOST="${LISTING_SYNC_HOST:-}"
LISTING_SYNC_PORT="${LISTING_SYNC_PORT:-80}"
LISTING_SYNC_PATH="${LISTING_SYNC_PATH:-/pwcats/ingest.php}"
LISTING_SYNC_SECRET="${LISTING_SYNC_SECRET:-}"

export LISTING_SYNC_PORT
export LISTING_SYNC_PATH
if [ -n "$LISTING_SYNC_HOST" ]; then
	export LISTING_SYNC_HOST
fi
if [ -n "$LISTING_SYNC_SECRET" ]; then
	export LISTING_SYNC_SECRET
fi
# --------------------------------------------------------------------------------------

ServerDir=root/pwserver
GsLogFileDir=$ServerDir
GsLogFileName=GameService_Log

if [ ! -d /$ServerDir/logs/ ]; then
mkdir /$ServerDir/logs/
fi
echo -e "=== [${txtred} START ${txtnrm}] Log Service ==="
cd /$ServerDir/logservice; ./logservice logservice.conf > /$ServerDir/logs/logservice.log &
echo -e "=== [${txtgrn} OK ${txtnrm}] ==="
echo -e ""

echo -e "=== [${txtred} START ${txtnrm}] Auth ==="
cd /$ServerDir/gauthd; ./gauthd gauthd.conf > /$ServerDir/logs/gauthd.log 2>&1 &
echo -e "=== [${txtgrn} OK ${txtnrm}] ==="
echo -e ""
#sleep 2

echo -e "=== [${txtred} START ${txtnrm}] Unique Name ==="
cd /$ServerDir/uniquenamed; ./uniquenamed gamesys.conf > /$ServerDir/logs/uniquenamed.log &
echo -e "=== [${txtgrn} OK ${txtnrm}] ==="
echo -e ""
#sleep 2

echo -e "=== [${txtred} START ${txtnrm}] Data Base ==="
cd /$ServerDir/gamedbd; ./gamedbd gamesys.conf > /$ServerDir/logs/gamedbd.log &
echo -e "=== [${txtgrn} OK ${txtnrm}] ==="
echo -e ""
#sleep 2
echo -e "=== [${txtred} START ${txtnrm}] Anti Cheat ==="
cd /$ServerDir/gacd; ./gacd gamesys.conf > /$ServerDir/logs/gacd.log &
echo -e "=== [${txtgrn} OK ${txtnrm}] ==="
echo -e ""
#sleep 2
echo -e "=== [${txtred} START ${txtnrm}] Faction ==="
cd /$ServerDir/gfactiond; ./gfactiond gamesys.conf > /$ServerDir/logs/gfactiond.log &
echo -e "=== [${txtgrn} OK ${txtnrm}] ==="
echo -e ""
#sleep 2
echo -e "=== [${txtred} START ${txtnrm}] Delivery ==="
cd /$ServerDir/gdeliveryd; ./gdeliveryd gamesys.conf > /$ServerDir/logs/gdeliveryd.log &
echo -e "=== [${txtgrn} OK ${txtnrm}] ==="
echo -e ""
#sleep 2
echo -e "=== [${txtred} START ${txtnrm}] Link ==="
cd /$ServerDir/glinkd; ./glinkd gamesys.conf 1 >> /$ServerDir/logs/glink1.log 2>&1 &
cd /$ServerDir/glinkd; ./glinkd gamesys.conf 2 >> /$ServerDir/logs/glink1.log 2>&1 &
cd /$ServerDir/glinkd; ./glinkd gamesys.conf 3 >> /$ServerDir/logs/glink1.log 2>&1 &
cd /$ServerDir/glinkd; ./glinkd gamesys.conf 4 >> /$ServerDir/logs/glink1.log 2>&1 &
echo -e "=== [${txtgrn} OK ${txtnrm}] ==="
echo -e ""
#sleep 2
echo -e "=== [${txtred} START ${txtnrm}] Game Service ==="
cd /$ServerDir/gamed
# libtask.so должна лежать рядом с gs; ld.so не ищет .so в "." без RPATH/LD_LIBRARY_PATH
export LD_LIBRARY_PATH="/$ServerDir/gamed:${LD_LIBRARY_PATH:-}"
echo -e "=== [${txtgrn} OK ${txtnrm}] ==="
echo -e ""
#sleep 2
./gs gs01 gs.conf gmserver.conf gsalias.conf > /$ServerDir/logs/gs.log 2>&1 &
echo "Instances started"
sleep 2
echo "is62"
cd /$ServerDir/gamed && export LD_LIBRARY_PATH="/$ServerDir/gamed:${LD_LIBRARY_PATH:-}" && ./gs is61 > /$ServerDir/logs/is61.log 2>&1 &
sleep 2


