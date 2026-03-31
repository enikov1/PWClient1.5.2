<?php
/**
 * Тестовая панель PW listing sync (PHP 7.4, без Node).
 * Пример развёртывания: http://192.168.159.135/pwcats/ → ingest: .../pwcats/ingest.php
 */
header('Content-Type: text/html; charset=utf-8');
$script = isset($_SERVER['SCRIPT_NAME']) ? str_replace('\\', '/', $_SERVER['SCRIPT_NAME']) : '/index.php';
$basePath = rtrim(dirname($script), '/');
if ($basePath === '/' || $basePath === '.') {
    $basePath = '';
}
$ingestUrl = htmlspecialchars(
    (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? 'https' : 'http')
    . '://' . ($_SERVER['HTTP_HOST'] ?? 'localhost')
    . $basePath
    . '/ingest.php',
    ENT_QUOTES,
    'UTF-8'
);
$jsBase = ($basePath === '' || $basePath === '.') ? '' : (rtrim($basePath, '/') . '/');
$syncSecret = getenv('LISTING_SYNC_SECRET');
if ($syncSecret === false || $syncSecret === '') {
    $syncSecret = '';
}
?>
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Listing Sync — тест</title>
  <style>
    :root {
      --bg: #0f1419;
      --panel: #1a2332;
      --border: #2d3d52;
      --text: #e6edf3;
      --muted: #8b9cb3;
      --accent: #58a6ff;
      --sell: #3fb950;
      --buy: #79c0ff;
      --auction: #d2a8ff;
      --comm: #ffa657;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Segoe UI", system-ui, sans-serif;
      background: var(--bg);
      color: var(--text);
      line-height: 1.45;
      font-size: 14px;
    }
    header {
      padding: 1rem 1.25rem;
      background: var(--panel);
      border-bottom: 1px solid var(--border);
    }
    h1 { margin: 0 0 0.35rem; font-size: 1.25rem; font-weight: 600; }
    .sub { color: var(--muted); font-size: 0.85rem; max-width: 900px; }
    main { padding: 1rem 1.25rem; max-width: 1400px; margin: 0 auto; }
    .toolbar {
      display: flex;
      flex-wrap: wrap;
      gap: 0.5rem;
      align-items: center;
      margin-bottom: 1rem;
    }
    button, select {
      background: var(--panel);
      border: 1px solid var(--border);
      color: var(--text);
      padding: 0.45rem 0.85rem;
      border-radius: 6px;
      cursor: pointer;
      font-size: 13px;
    }
    button:hover { border-color: var(--accent); }
    button.primary { background: #1f6feb; border-color: #388bfd; }
    .pill {
      display: inline-block;
      padding: 0.15rem 0.5rem;
      border-radius: 999px;
      font-size: 11px;
      font-weight: 600;
      text-transform: uppercase;
    }
    .pill.cat { background: #238636; color: #fff; }
    .pill.commission { background: #9e6a03; color: #fff; }
    .pill.auction { background: #8957e5; color: #fff; }
    table {
      width: 100%;
      border-collapse: collapse;
      background: var(--panel);
      border-radius: 8px;
      overflow: hidden;
      border: 1px solid var(--border);
    }
    th, td {
      text-align: left;
      padding: 0.55rem 0.65rem;
      border-bottom: 1px solid var(--border);
      vertical-align: top;
    }
    th { background: #212c3d; color: var(--muted); font-weight: 600; font-size: 12px; }
    tr:last-child td { border-bottom: none; }
    tr:hover td { background: rgba(88, 166, 255, 0.06); }
    .mono { font-family: ui-monospace, Consolas, monospace; font-size: 12px; }
    .coords { font-weight: 500; }
    .coords.cat { color: var(--buy); }
    .coords.commission { color: var(--comm); }
    .coords.auction { color: var(--auction); }
    .goods-cell { max-width: 420px; overflow: visible; }
    table { overflow: visible; }
    .goods-entry {
      position: relative;
      margin: 0.15rem 0;
    }
    .goods-entry:hover {
      z-index: 20;
    }
    .goods-line { margin: 0; }
    .goods-line.side-sell { cursor: default; }
    .goods-tooltip-body {
      display: none;
      position: absolute;
      left: 100%;
      top: 0;
      margin-left: 10px;
      z-index: 100;
      min-width: 280px;
      max-width: min(440px, 85vw);
      box-shadow: 0 8px 24px rgba(0, 0, 0, 0.45);
      border-radius: 8px;
    }
    .goods-entry:hover .goods-tooltip-body {
      display: block;
    }
    .goods-hover-hint {
      font-size: 11px;
      color: var(--muted);
      margin-top: 2px;
    }
    .side-sell { color: var(--sell); }
    .side-buy { color: var(--buy); }
    pre.raw {
      margin: 1rem 0 0;
      padding: 1rem;
      background: #0d1117;
      border: 1px solid var(--border);
      border-radius: 8px;
      overflow: auto;
      max-height: 240px;
      font-size: 11px;
    }
    .hint {
      margin-top: 1rem;
      padding: 0.85rem 1rem;
      background: rgba(88, 166, 255, 0.08);
      border: 1px solid var(--border);
      border-radius: 8px;
      font-size: 12px;
      color: var(--muted);
    }
    .hint code { color: var(--accent); }
  </style>
  <script>
    window.PWCATS_LISTING_SYNC_SECRET = <?php echo json_encode($syncSecret, JSON_HEX_TAG | JSON_HEX_AMP | JSON_HEX_APOS | JSON_HEX_QUOT); ?>;
    window.PWCATS_BASE = <?php echo json_encode($jsBase); ?>;
  </script>
</head>
<body>
  <header>
    <h1>Listing sync — тестовая панель (PHP 7.4)</h1>
    <p class="sub">
      События от gameserver (коты) и gdeliveryd (комиссионка, аукцион). Колонка «Где»: кот — координаты и
      <span class="mono">world_tag</span>; комиссионка — текст «Комиссионный магазин»; аукцион — номер лота.
    </p>
  </header>
  <main>
    <div class="toolbar">
      <label>
        Источник
        <select id="filterSource">
          <option value="">Все</option>
          <option value="cat">Кот (cat)</option>
          <option value="commission">Комиссионка</option>
          <option value="auction">Аукцион</option>
        </select>
      </label>
      <button type="button" id="btnRefresh">Обновить</button>
      <button type="button" id="btnClear" class="primary">Очистить лог</button>
      <span id="status" class="mono" style="color:var(--muted)"></span>
    </div>
    <p class="mono" style="color:var(--muted);font-size:12px;margin:0 0 0.75rem">
      Endpoint для GS (ниже подставляется автоматически от текущего хоста и пути):<br />
      <strong style="color:var(--accent)"><?php echo $ingestUrl; ?></strong>
      <span style="display:block;margin-top:0.35rem">При запуске с <code>http://192.168.159.135/pwcats/</code> в <code>gs.conf</code> укажите
      <code class="mono">url=http://192.168.159.135/pwcats/ingest.php</code>.</span>
      <span style="display:block;margin-top:0.5rem">Характеристики вещи в таблице (заточка, камни, статы) приходят из <code>item_save_hex</code> в JSON лавки;
      <code>listings.php</code> собирает HTML-тултип через <code>PwItemTooltip</code> и кэш имён <code>data/elements_names.json</code>. Для своего фронта: <code>listings.php?decode=2</code> → поле <code>detail.tooltip_json</code>.</span>
    </p>
    <table>
      <thead>
        <tr>
          <th>#</th>
          <th>Время</th>
          <th>Тип</th>
          <th>Действие</th>
          <th>Торговец / роль</th>
          <th>Название / детали</th>
          <th>Товары</th>
          <th>Где (как на сайте)</th>
        </tr>
      </thead>
      <tbody id="rows"></tbody>
    </table>
    <div class="hint">
      <strong>Сервер:</strong> PHP 7.4+, расширение <code>json</code>. Данные в <code>pwcats/data/listings.json</code>
      (каталог закрыт от HTTP через <code>.htaccess</code> на Apache).<br />
      <strong>Gameserver (Linux, <code>cgame/gs</code>):</strong> в <code>gs.conf</code> секция
      <code>[StallWebsiteSync]</code>: <code>enabled=true</code>,
      <code>url=http://192.168.159.135/pwcats/ingest.php</code> (или тот же URL, что в строке «Endpoint» выше),
      <code>secret</code> = тому же значению, что <code>LISTING_SYNC_SECRET</code> в PHP (заголовок
      <code>X-Stall-Sync-Secret</code> — принимается наряду с <code>X-Listing-Sync-Secret</code>).<br />
      <strong>Другие способы (если не gs.conf):</strong>
      <code>export LISTING_SYNC_HOST=ваш.хост</code>,
      <code>export LISTING_SYNC_PORT=80</code>,
      <code>export LISTING_SYNC_PATH=/pwcats/ingest.php</code> (для <code>192.168.159.135</code> хост и порт 80).
      Опционально секрет в PHP: <code>SetEnv LISTING_SYNC_SECRET ...</code>
      (или <code>env[LISTING_SYNC_SECRET]=...</code> в pool).
      Кнопки «Пример» подставляют тот же секрет со страницы
      (ограничьте доступ к <code>index.php</code>, иначе секрет виден в HTML). Если секрет только в игре, а в PHP не задан —
      примеры работают, а посты без заголовка принимаются только пока в окружении PHP секрет пустой.<br />
      Клик по строке таблицы — сырой JSON внизу.
    </div>
    <p style="margin-top:0.75rem">
      <button type="button" id="sampleCat">Пример: кот (open)</button>
      <button type="button" id="sampleComm">Пример: комиссионка</button>
      <button type="button" id="sampleAuc">Пример: аукцион</button>
    </p>
    <pre class="raw" id="rawJson" hidden></pre>
  </main>
  <script>
    (function () {
      var base = typeof window.PWCATS_BASE === 'string' ? window.PWCATS_BASE : '';
      function ingestHeaders() {
        var h = { 'Content-Type': 'application/json' };
        if (window.PWCATS_LISTING_SYNC_SECRET) {
          h['X-Listing-Sync-Secret'] = window.PWCATS_LISTING_SYNC_SECRET;
        }
        return h;
      }
      function hexToUtf16le(hex) {
        if (!hex || typeof hex !== 'string') return '';
        var clean = hex.replace(/\s/g, '');
        if (clean.length % 4 !== 0) return '(hex не UTF-16LE?)';
        var u8 = new Uint8Array(clean.length / 2);
        for (var i = 0; i < clean.length; i += 2) {
          u8[i / 2] = parseInt(clean.slice(i, i + 2), 16);
        }
        try {
          return new TextDecoder('utf-16le').decode(u8);
        } catch (e) {
          return hex;
        }
      }
      function escapeHtml(s) {
        return String(s)
          .replace(/&/g, '&amp;')
          .replace(/</g, '&lt;')
          .replace(/>/g, '&gt;')
          .replace(/"/g, '&quot;');
      }
      function fmtItemDetail(d, stallSlot) {
        if (!d || typeof d !== 'object') return '';
        var out = [];
        var title = '';
        if (d.item_name_hex) {
          title = hexToUtf16le(d.item_name_hex) || '';
        }
        if (title) {
          out.push(
            '<div style="margin-top:6px;font-weight:600">' + escapeHtml(title) + '</div>'
          );
        }
        if (d.tooltip_html) {
          out.push(
            '<div class="goods-hover-hint">Наведите курсор — характеристики и камни.</div>' +
              '<div class="goods-tooltip-body">' +
              String(d.tooltip_html) +
              '</div>'
          );
        }
        (function () {
          var slotPart = '';
          if (stallSlot != null && stallSlot !== '' && !isNaN(Number(stallSlot))) {
            slotPart = 'слот лавки ' + (Number(stallSlot) + 1);
          } else if (d.inv_index != null) {
            slotPart =
              'инв. ' +
              d.inv_index +
              ' <span style="opacity:0.75">(в JSON нет slot лавки)</span>';
          } else {
            slotPart = 'слот ?';
          }
          out.push(
            '<div class="mono" style="font-size:11px;margin-top:2px;color:var(--muted)">' +
              slotPart +
              ' · equip_mask 0x' +
              (d.equip_mask != null ? (Number(d.equip_mask) >>> 0).toString(16) : '?') +
              ' · proc 0x' +
              (d.proc_type != null ? (Number(d.proc_type) >>> 0).toString(16) : '?') +
              '</div>'
          );
        })();
        if (d.inv_count != null || d.pile_limit != null) {
          out.push(
            '<div class="mono" style="font-size:11px;color:var(--muted)">в рюкзаке: ' +
              (d.inv_count != null ? d.inv_count : '?') +
              ' / лимит ' +
              (d.pile_limit != null ? d.pile_limit : '?') +
              '</div>'
          );
        }
        if (d.expire_date != null && d.expire_date > 0) {
          out.push(
            '<div class="mono" style="font-size:11px;color:var(--muted)">срок: ' + d.expire_date + '</div>'
          );
        }
        if (d.item_save_truncated) {
          out.push(
            '<div style="color:var(--comm);font-size:11px">Снимок не влез в лимит GS, байт: ' +
              (d.item_save_size != null ? d.item_save_size : '?') +
              '</div>'
          );
        } else if (d.item_save_hex && d.item_save_hex.length) {
          out.push(
            '<details style="margin-top:4px"><summary style="cursor:pointer;font-size:11px;color:var(--accent)">Полный снимок предмета (заточка, камни, аддоны) — ' +
              (d.item_save_hex.length / 2) +
              ' байт</summary>' +
              '<textarea readonly class="mono" style="margin-top:4px;width:100%;min-height:80px;max-height:220px;font-size:10px;background:#0d1117;color:var(--text);border:1px solid var(--border);border-radius:4px;padding:6px;box-sizing:border-box">' +
              escapeHtml(d.item_save_hex) +
              '</textarea>' +
              '<p class="mono" style="font-size:10px;color:var(--muted);margin:6px 0 0;line-height:1.35">' +
              'Поток <code>item::Save</code> с GS. Над блоком — уже разобранный тултип: статы essence, гнёзда (id камней → имена из <code>data/elements_names.json</code>), аддоны с аргументами. ' +
              'Полный JSON разбора: запрос <code>listings.php?decode=2</code> → <code>detail.tooltip_json</code>.</p></details>'
          );
        }
        return (
          '<div class="goods-detail-block" style="margin:6px 0 2px 8px;padding-left:8px;border-left:2px solid var(--border)">' +
          out.join('') +
          '</div>'
        );
      }
      function fmtGoods(body) {
        var lines = [];
        if (body.listing_source === 'cat' && Array.isArray(body.goods)) {
          body.goods.forEach(function (g) {
            var side = g.side;
            if (!side && g.is_buy_order != null) {
              side = Number(g.is_buy_order) === 1 ? 'buy' : 'sell';
            }
            var cls = side === 'buy' ? 'side-buy' : 'side-sell';
            var row =
              '<div class="goods-entry">' +
              '<div class="goods-line ' + cls + '">' +
              (side === 'buy' ? 'куплю' : 'продам') +
              ' id ' +
              g.item_id +
              ' × ' +
              g.count +
              ' @ ' +
              Number(g.price).toLocaleString('ru-RU') +
              '</div>';
            if (g.detail && side !== 'buy') {
              row += fmtItemDetail(g.detail, g.slot);
            }
            row += '</div>';
            lines.push(row);
          });
        }
        if (body.listing_source === 'commission') {
          if (Array.isArray(body.sell)) {
            body.sell.forEach(function (g) {
              lines.push(
                '<div class="goods-line side-sell">продам id ' + g.item_id + ' × ' + g.count +
                  ' @ ' + Number(g.price).toLocaleString('ru-RU') + '</div>'
              );
            });
          }
          if (Array.isArray(body.buy)) {
            body.buy.forEach(function (g) {
              lines.push(
                '<div class="goods-line side-buy">куплю id ' + g.item_id + ' × ' + g.count +
                  ' @ ' + Number(g.price).toLocaleString('ru-RU') + '</div>'
              );
            });
          }
        }
        if (body.listing_source === 'auction') {
          lines.push(
            '<div class="goods-line">item ' + body.item_id + ' × ' + body.item_count +
              ' — цена ' + Number(body.price).toLocaleString('ru-RU') + '</div>'
          );
        }
        return lines.length ? lines.join('') : '—';
      }
      function colWhere(body) {
        var src = body.listing_source || '';
        if (src === 'cat') {
          var tag = body.world_tag != null ? body.world_tag : '?';
          var mm = body.pw_minimap;
          if (mm && mm.x != null && mm.z != null) {
            var yPart = mm.y != null ? (' y=' + mm.y) : '';
            var rawX = body.pos && body.pos.x != null ? Number(body.pos.x).toFixed(1) : null;
            var rawZ = body.pos && body.pos.z != null ? Number(body.pos.z).toFixed(1) : null;
            var rawLine =
              rawX != null && rawZ != null
                ? '<br><span class="mono" style="color:var(--muted)">мир XYZ: ' +
                  rawX + ', ' +
                  (body.pos.y != null ? Number(body.pos.y).toFixed(1) : '?') +
                  ', ' + rawZ + ' · world_tag=' + tag + '</span>'
                : '<br><span class="mono" style="color:var(--muted)">world_tag=' + tag + '</span>';
            return (
              '<span class="coords cat">' + mm.x + ' ' + mm.z + yPart + '</span> ' +
              '<span class="mono" style="color:var(--muted)">(как в клиенте)</span>' +
              rawLine
            );
          }
          var px = body.pos_x != null ? body.pos_x : (body.pos && body.pos.x != null ? body.pos.x : null);
          var pz = body.pos_z != null ? body.pos_z : (body.pos && body.pos.z != null ? body.pos.z : null);
          var x = px != null ? Number(px).toFixed(0) : '?';
          var z = pz != null ? Number(pz).toFixed(0) : '?';
          return (
            '<span class="coords cat">' + x + ' ' + z + ' · world_tag=' + tag + '</span><br>' +
            '<span class="mono" style="color:var(--muted)">(мировые; после обновления GS будут ещё pw_minimap как в клиенте)</span>'
          );
        }
        if (src === 'commission') {
          return '<span class="coords commission">Комиссионный магазин</span>';
        }
        if (src === 'auction') {
          return '<span class="coords auction">Аукцион лот № ' + (body.sn != null ? body.sn : '?') + '</span>';
        }
        return '—';
      }
      function colTrader(body) {
        if (body.listing_source === 'cat') {
          var rid = body.role_id != null ? body.role_id : body.roleid;
          return '<span class="mono">role ' + (rid != null ? rid : '?') + '</span>';
        }
        if (body.listing_source === 'commission') {
          return '<span class="mono">role ' + (body.role_id != null ? body.role_id : '?') + '</span>';
        }
        if (body.listing_source === 'auction') {
          return '<span class="mono">seller ' + (body.seller_roleid != null ? body.seller_roleid : '?') + '</span>';
        }
        return '—';
      }
      function colTitle(body) {
        if (body.listing_source === 'cat') {
          var snHex = body.shop_name_hex || body.stall_name_hex;
          var name = snHex ? hexToUtf16le(snHex) : '';
          var rname = body.role_name_hex ? hexToUtf16le(body.role_name_hex) : '';
          return (
            (name ? '<div>Лавка: <strong>' + escapeHtml(name) + '</strong></div>' : '') +
            (rname ? '<div class="mono" style="margin-top:4px;color:var(--muted)">ник: ' +
              escapeHtml(rname) + '</div>' : '')
          );
        }
        if (body.listing_source === 'commission') {
          return (
            '<div class="mono">shoptype ' + (body.shoptype != null ? body.shoptype : '?') +
              ' · money ' + (body.money != null ? Number(body.money).toLocaleString('ru-RU') : '?') + '</div>'
          );
        }
        if (body.listing_source === 'auction') {
          var sn = body.seller_name_hex ? hexToUtf16le(body.seller_name_hex) : '';
          return (
            '<div class="mono">sn=' + body.sn + ' state=' + body.state + ' posttype=' + body.posttype + '</div>' +
            (sn ? '<div style="margin-top:4px">' + escapeHtml(sn) + '</div>' : '')
          );
        }
        return '—';
      }
      function pill(body) {
        var s = body.listing_source || '?';
        return '<span class="pill ' + escapeHtml(s) + '">' + escapeHtml(s) + '</span>';
      }
      function load() {
        var src = document.getElementById('filterSource').value;
        var st = document.getElementById('status');
        st.textContent = 'загрузка…';
        var q = src ? '?source=' + encodeURIComponent(src) : '';
        fetch(base + 'listings.php' + q)
          .then(function (r) { return r.json(); })
          .then(function (data) {
            var tb = document.getElementById('rows');
            tb.innerHTML = '';
            if (!data.items || !data.items.length) {
              tb.innerHTML =
                '<tr><td colspan="8" style="color:var(--muted)">Нет событий. Используйте игру или «Пример».</td></tr>';
              st.textContent = '0 записей';
              return;
            }
            data.items.forEach(function (row) {
              var b = row.body;
              var tr = document.createElement('tr');
              tr.innerHTML =
                '<td class="mono">' + row.id + '</td>' +
                '<td class="mono">' + escapeHtml(String(row.at).replace('T', ' ').slice(0, 19)) + '</td>' +
                '<td>' + pill(b) + '</td>' +
                '<td class="mono">' + escapeHtml(b.action || b.event || '') + '</td>' +
                '<td>' + colTrader(b) + '</td>' +
                '<td>' + colTitle(b) + '</td>' +
                '<td class="goods-cell">' + fmtGoods(b) + '</td>' +
                '<td>' + colWhere(b) + '</td>';
              tr.addEventListener('click', function () {
                var pre = document.getElementById('rawJson');
                pre.hidden = false;
                pre.textContent = JSON.stringify(b, null, 2);
              });
              tb.appendChild(tr);
            });
            st.textContent = data.items.length + ' записей';
          })
          .catch(function (e) {
            st.textContent = 'ошибка: ' + e.message;
          });
      }
      function postSample(obj) {
        fetch(base + 'ingest.php', {
          method: 'POST',
          headers: ingestHeaders(),
          body: JSON.stringify(obj),
        })
          .then(function (r) {
            if (!r.ok) {
              return r.text().then(function (t) {
                var msg = r.status + ' ' + t;
                if (r.status === 401) {
                  msg += ' (проверьте LISTING_SYNC_SECRET в окружении PHP — он должен совпадать с тем, что в странице)';
                }
                throw new Error(msg);
              });
            }
            return r.json();
          })
          .then(function () { load(); })
          .catch(function (e) { alert(e.message); });
      }
      document.getElementById('btnRefresh').addEventListener('click', load);
      document.getElementById('filterSource').addEventListener('change', load);
      document.getElementById('btnClear').addEventListener('click', function () {
        if (!confirm('Очистить все записи?')) return;
        fetch(base + 'clear.php', { method: 'POST' }).then(function () {
          document.getElementById('rawJson').hidden = true;
          load();
        });
      });
      document.getElementById('sampleCat').addEventListener('click', function () {
        postSample({
          listing_source: 'cat',
          action: 'open',
          role_id: 12345,
          stall_trade_id: 1,
          world_tag: 1,
          pos_x: 528,
          pos_y: 0,
          pos_z: 665,
          pw_minimap: { x: 452, z: 616, y: 0 },
          shop_name_hex:
            '3c0030003c0020000f003e003e002000cf0448043004380043003900200436043804380043003e000f003c002f003e00',
          role_name_hex: '540065007300740050006c006100790065007200',
          goods: [
            { item_id: 1122, count: 1, price: 18000000, side: 'sell' },
            { item_id: 3044, count: 100, price: 1, side: 'buy' },
          ],
        });
      });
      document.getElementById('sampleComm').addEventListener('click', function () {
        postSample({
          listing_source: 'commission',
          action: 'update',
          role_id: 99901,
          shoptype: 1,
          money: 5000000,
          sell: [{ item_id: 44102, count: 2988, price: 1800000 }],
          buy: [{ item_id: 44102, count: 5000, price: 167100 }],
        });
      });
      document.getElementById('sampleAuc').addEventListener('click', function () {
        postSample({
          listing_source: 'auction',
          action: 'open',
          sn: 474880,
          seller_roleid: 555,
          posttype: 2,
          item_id: 44102,
          item_count: 100,
          price: 1800000,
          state: 2,
          seller_name_hex: '',
        });
      });
      load();
      setInterval(load, 5000);
    })();
  </script>
</body>
</html>
