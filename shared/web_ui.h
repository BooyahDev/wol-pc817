#pragma once

#include <sstream>
#include <string>
#include <vector>

// 共通のWeb UIを生成（日本語表示、軽量）
inline std::string buildWebUI(const std::vector<std::string>& pcNames,
                              const std::string& platformLabel,
                              const std::string& modeLabel,
                              int pcCount) {
    std::ostringstream html;

    // 1) ヘッダーと先頭部分
    html << R"HTML(<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PC電源制御</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; background: #f7f7f7; color: #222; padding: 16px; }
        .wrap { max-width: 760px; margin: 0 auto; }
        h1 { font-size: 1.6rem; margin-bottom: 12px; }
        .meta { font-size: 0.9rem; color: #555; margin-bottom: 16px; }
        .info { background: #fff; border: 1px solid #ddd; padding: 12px; margin-bottom: 12px; }
        .grid { display: grid; gap: 10px; grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); margin-bottom: 12px; }
        .card { background: #fff; border: 1px solid #ddd; padding: 12px; }
        .name { font-weight: 600; margin-bottom: 6px; }
        .btn { width: 100%; padding: 10px; margin-top: 6px; border: 1px solid #ccc; background: #f0f0f0; cursor: pointer; font-weight: 600; }
        .btn:active { background: #e0e0e0; }
        .message { margin-top: 8px; padding: 8px; display: none; border: 1px solid #ccc; background: #fafafa; }
    </style>
</head>
<body>
    <div class="wrap">
        <h1>PC電源制御</h1>
        <div class="meta">プラットフォーム: )HTML";

    // 2) プラットフォームとモード
    html << platformLabel << " / モード: " << modeLabel << R"HTML(</div>

        <div class="grid" id="pcGrid"></div>

        <div class="info">
            <div>接続PC台数: <span id="pcCount">)HTML";

    // 3) 台数
    html << pcCount << R"HTML(</span></div>
            <div id="message" class="message"></div>
        </div>
    </div>

    <script>
        const pcNames = [)HTML";

    // 4) PC名の配列を挿入
    for (size_t i = 0; i < pcNames.size(); ++i) {
        html << "\"" << pcNames[i] << "\"";
        if (i + 1 < pcNames.size()) html << ",";
    }

    // 5) 残りのスクリプト
    html << R"HTML(];

        function showMessage(text, isSuccess) {
            const msg = document.getElementById('message');
            msg.textContent = text;
            msg.className = 'message ' + (isSuccess ? 'success' : 'error');
            msg.style.display = 'block';
            setTimeout(() => { msg.style.display = 'none'; }, 3000);
        }

        async function togglePower(pcIndex) {
            try {
                const response = await fetch(`/api/power/${pcIndex}`, { method: 'POST' });
                const data = await response.json();
                if (response.ok) {
                    showMessage(data.message, true);
                } else {
                    showMessage('エラー: ' + data.message, false);
                }
            } catch (error) {
                showMessage('通信エラー: ' + error, false);
            }
        }

        async function longPressPower(pcIndex) {
            if (!confirm(`${pcNames[pcIndex]} を強制シャットダウンしますか？\n電源ボタンを5秒間長押しします。`)) return;
            try {
                showMessage(`${pcNames[pcIndex]} の電源ボタンを長押ししています...`, true);
                const response = await fetch(`/api/longpress/${pcIndex}`, { method: 'POST' });
                const data = await response.json();
                if (response.ok) {
                    showMessage(data.message, true);
                } else {
                    showMessage('エラー: ' + data.message, false);
                }
            } catch (error) {
                showMessage('通信エラー: ' + error, false);
            }
        }

        function updatePCCards() {
            const grid = document.getElementById('pcGrid');
            grid.innerHTML = '';
            pcNames.forEach((name, index) => {
                const card = document.createElement('div');
                card.className = 'card';
                card.innerHTML =
                    "<div class='name'>" + name + "</div>" +
                    "<button class='btn' onclick='togglePower(" + index + ")'>電源トグル</button>" +
                    "<button class='btn' onclick='longPressPower(" + index + ")'>強制シャットダウン (5秒)</button>";
                grid.appendChild(card);
            });
        }

        updatePCCards();
    </script>
</body>
</html>)HTML";

    return html.str();
}
