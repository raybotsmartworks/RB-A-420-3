#ifndef WEB_PAGES_H
#define WEB_PAGES_H

const char MAIN_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 机械臂控制</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { font-family: 'Segoe UI', Arial, sans-serif; background: #1a1a2e; color: #eee; min-height: 100vh; padding: 15px; }
        .header { text-align: center; padding: 10px; background: #16213e; border-radius: 10px; margin-bottom: 15px; }
        .header h1 { color: #00d9ff; font-size: 1.4em; }
        .tab-bar { display: flex; background: #16213e; border-radius: 8px; margin-bottom: 15px; overflow: hidden; }
        .tab { flex: 1; padding: 12px; text-align: center; cursor: pointer; background: #1f4068; color: #aaa; border: none; font-size: 14px; transition: all 0.3s; }
        .tab.active { background: #00d9ff; color: #1a1a2e; font-weight: bold; }
        .tab:hover:not(.active) { background: #2a5a8a; }
        .tab-content { display: none; background: #16213e; border-radius: 10px; padding: 15px; }
        .tab-content.active { display: block; }
        .status-bar { background: #0f3460; padding: 10px; border-radius: 8px; margin-bottom: 15px; text-align: center; font-size: 14px; color: #00d9ff; }
        .section-title { color: #00d9ff; font-size: 12px; margin: 10px 0 8px; padding-bottom: 5px; border-bottom: 1px solid #1f4068; }
        .btn-group { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; margin-bottom: 10px; }
        .btn-group-4 { grid-template-columns: repeat(4, 1fr); }
        .btn { padding: 12px 8px; border: none; border-radius: 8px; cursor: pointer; font-size: 13px; font-weight: bold; transition: all 0.2s; min-height: 44px; }
        .btn:active { transform: scale(0.95); }
        .btn-green { background: #00a854; color: white; }
        .btn-green:hover { background: #00c45e; }
        .btn-blue { background: #1976d2; color: white; }
        .btn-blue:hover { background: #2196f3; }
        .btn-orange { background: #ff9800; color: white; }
        .btn-orange:hover { background: #ffa726; }
        .btn-red { background: #d32f2f; color: white; }
        .btn-red:hover { background: #f44336; }
        .btn-purple { background: #7b1fa2; color: white; }
        .btn-purple:hover { background: #9c27b0; }
        .btn-teal { background: #00897b; color: white; }
        .btn-teal:hover { background: #00a08a; }
        .btn-gray { background: #455a64; color: white; }
        .btn-gray:hover { background: #546e7a; }
        .angle-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 8px; margin-bottom: 15px; }
        .angle-item { background: #0f3460; padding: 10px; border-radius: 8px; text-align: center; }
        .angle-label { font-size: 11px; color: #aaa; margin-bottom: 3px; }
        .angle-value { font-size: 16px; font-weight: bold; color: #00d9ff; }
        .btn-wide { grid-column: span 2; }
        .flash-info { font-size: 12px; color: #aaa; margin-top: 8px; text-align: center; }
        .teach-count { font-size: 12px; color: #00d9ff; text-align: center; margin-bottom: 10px; }
    </style>
</head>
<body>
    <div class="header">
        <h1>ESP32 六轴机械臂控制台</h1>
    </div>

    <div class="tab-bar">
        <button class="tab active" onclick="showTab('monitor')">状态监控</button>
        <button class="tab" onclick="showTab('teach')">示教</button>
        <button class="tab" onclick="showTab('preset')">预设动作</button>
        <button class="tab" onclick="showTab('safety')">安全与工具</button>
    </div>

    <div id="monitor" class="tab-content active">
        <div class="status-bar" id="statusDisplay">等待指令</div>
        <div class="angle-grid">
            <div class="angle-item"><div class="angle-label">J1</div><div class="angle-value" id="j1">--</div></div>
            <div class="angle-item"><div class="angle-label">J2</div><div class="angle-value" id="j2">--</div></div>
            <div class="angle-item"><div class="angle-label">J3</div><div class="angle-value" id="j3">--</div></div>
            <div class="angle-item"><div class="angle-label">J4</div><div class="angle-value" id="j4">--</div></div>
            <div class="angle-item"><div class="angle-label">J5</div><div class="angle-value" id="j5">--</div></div>
            <div class="angle-item"><div class="angle-label">J6</div><div class="angle-value" id="j6">--</div></div>
            <div class="angle-item"><div class="angle-label">夹爪</div><div class="angle-value" id="claw">--</div></div>
        </div>
        <div class="btn-group">
            <button class="btn btn-blue" onclick="sendCmd('readJoints')">读取角度</button>
        </div>
    </div>

    <div id="teach" class="tab-content">
        <div class="teach-count" id="teachCount">已记录: 0/10</div>
        <div class="section-title">基本操作</div>
        <div class="btn-group">
            <button class="btn btn-green" onclick="sendCmd('record')">记录</button>
            <button class="btn btn-green" onclick="sendCmd('runTeach')">执行</button>
            <button class="btn btn-orange" onclick="sendCmd('clearTeach')">清空</button>
        </div>
        <div class="section-title">Flash存储</div>
        <div class="btn-group">
            <button class="btn btn-blue" onclick="sendCmd('saveFlash')">保存</button>
            <button class="btn btn-blue" onclick="sendCmd('loadFlash')">加载</button>
            <button class="btn btn-red" onclick="sendCmd('deleteFlash')">删除</button>
        </div>
        <div class="section-title">夹爪控制</div>
        <div class="btn-group">
            <button class="btn btn-teal" onclick="sendCmd('clawOpen')">张开</button>
            <button class="btn btn-teal" onclick="sendCmd('clawClose')">闭合</button>
        </div>
        <div class="section-title">写入预设</div>
        <div class="btn-group btn-group-4">
            <button class="btn btn-purple" onclick="sendCmd('savePreset1')">预设1</button>
            <button class="btn btn-purple" onclick="sendCmd('savePreset2')">预设2</button>
            <button class="btn btn-purple" onclick="sendCmd('savePreset3')">预设3</button>
            <button class="btn btn-purple" onclick="sendCmd('savePreset4')">预设4</button>
        </div>
        <div class="flash-info" id="flashInfo">Flash: 未保存示教组</div>
        <div class="section-title">紧急操作</div>
        <div class="btn-group">
            <button class="btn btn-red btn-wide" onclick="sendCmd('emergency')">紧急停止</button>
        </div>
    </div>

    <div id="preset" class="tab-content">
        <div class="section-title">运行预设</div>
        <div class="btn-group btn-group-4">
            <button class="btn btn-purple" onclick="sendCmd('preset1')">运行1</button>
            <button class="btn btn-purple" onclick="sendCmd('preset2')">运行2</button>
            <button class="btn btn-purple" onclick="sendCmd('preset3')">运行3</button>
            <button class="btn btn-purple" onclick="sendCmd('preset4')">运行4</button>
        </div>
        <div class="section-title">紧急操作</div>
        <div class="btn-group">
            <button class="btn btn-red btn-wide" onclick="sendCmd('emergency')">紧急停止</button>
        </div>
    </div>

    <div id="safety" class="tab-content">
        <div class="section-title">安全操作</div>
        <div class="btn-group">
            <button class="btn btn-orange btn-wide" onclick="sendCmd('setZero')">全关节置零</button>
        </div>
    </div>

    <script>
        function showTab(name) {
            document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            document.getElementById(name).classList.add('active');
            event.target.classList.add('active');
        }

        function sendCmd(cmd) {
            var xhttp = new XMLHttpRequest();
            xhttp.open('GET', '/cmd?action=' + cmd, true);
            xhttp.send();
        }

        function updateStatus(text) {
            document.getElementById('statusDisplay').textContent = text;
        }

        function updateAngles(angles) {
            for (var i = 0; i < 6; i++) {
                document.getElementById('j' + (i+1)).textContent = angles[i] || '--';
            }
            document.getElementById('claw').textContent = angles[6] || '--';
        }

        function updateTeachCount(count) {
            document.getElementById('teachCount').textContent = '已记录: ' + count + '/10';
        }

        function updateFlashInfo(info) {
            document.getElementById('flashInfo').textContent = info;
        }

        setInterval(function() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    try {
                        var data = JSON.parse(this.responseText);
                        if (data.status) updateStatus(data.status);
                        if (data.angles) updateAngles(data.angles);
                        if (data.teachCount !== undefined) updateTeachCount(data.teachCount);
                        if (data.flashInfo) updateFlashInfo(data.flashInfo);
                    } catch(e) {}
                }
            };
            xhttp.open('GET', '/status', true);
            xhttp.send();
        }, 1000);
    </script>
</body>
</html>
)=====";

#endif
