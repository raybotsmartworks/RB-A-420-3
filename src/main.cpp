#include "rclib.h"
#include "web_pages.h"

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <math.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <esp_task_wdt.h>

IPAddress apIP(192, 168, 4, 1);

const char *ssid = "脉塔智能";
const char *password = "maita666";

static const int kJointCount = 6;
static const int kJointStartId = 1;
static const int kGripperId = 7;
static const int kDefaultSpeed = 60;
static const int kTeachMaxPose = 10;
static const int kPoseSendGapMs = 20;

struct Pose
{
    int16_t joint[kJointCount];
    uint16_t holdMs;
};

struct RuntimeSequence
{
    const Pose *poses;
    uint8_t count;
    uint8_t index;
    bool running;
    unsigned long nextStepMs;
    String name;
};

Pose teachGroup[kTeachMaxPose];
uint8_t teachCount = 0;
Preferences preferences;
Pose presetGroups[4][kTeachMaxPose];
uint8_t presetCounts[4] = {0, 0, 0, 0};

RuntimeSequence seq = {nullptr, 0, 0, false, 0, ""};

String currentStatus = "等待指令";
String flashInfoText = "Flash: 未保存示教组";
float currentAngles[7] = {0};
AsyncWebServer server(80);

enum class JointReadState { IDLE, READING_JOINTS, READING_CLAW, DONE };
JointReadState jointReadState = JointReadState::IDLE;
int jointReadIndex = 0;

static const uint32_t kTeachMagic = 0x54454348;
static const uint16_t kTeachVersion = 1;
static const uint32_t kPresetMagic = 0x50524553;
static const uint16_t kPresetVersion = 1;

struct TeachStoreBlob
{
    uint32_t magic;
    uint16_t version;
    uint8_t count;
    uint8_t reserved;
    Pose poses[kTeachMaxPose];
    uint32_t checksum;
};

struct PresetStoreBlob
{
    uint32_t magic;
    uint16_t version;
    uint8_t counts[4];
    uint8_t reserved;
    Pose poses[4][kTeachMaxPose];
    uint32_t checksum;
};

void updateTeachCountLabel();
void saveTeachToPreset(uint8_t presetIndex);

const Pose preset1Default[] = {
    {{0, -45, -30, 0, -30, 0}, 1400},
    {{0, 30, 0, 0, 45, 0}, 1400}};

const Pose preset2Default[] = {
    {{-40, -50, -60, 0, -40, -90}, 1200},
    {{-40, -20, -30, 0, -20, -90}, 1200},
    {{60, -20, -30, 0, -20, 0}, 1200},
    {{60, -50, -60, 0, -40, 0}, 1200}};

const Pose preset3Default[] = {
    {{20, -20, -10, 0, 20, 30}, 900},
    {{-20, -20, -10, 0, 20, -30}, 900},
    {{0, 0, 0, 0, 0, 0}, 900}};

const Pose preset4Default[] = {
    {{0, -35, -20, 10, 30, 0}, 1000},
    {{30, -25, -25, -10, 15, 45}, 1000},
    {{-30, -25, -25, -10, 15, -45}, 1000},
    {{0, 0, 0, 0, 0, 0}, 1000}};

void copyPoseList(Pose *dst, const Pose *src, uint8_t count)
{
    for (uint8_t i = 0; i < count; ++i)
    {
        dst[i] = src[i];
    }
}

void initDefaultPresets()
{
    presetCounts[0] = sizeof(preset1Default) / sizeof(preset1Default[0]);
    presetCounts[1] = sizeof(preset2Default) / sizeof(preset2Default[0]);
    presetCounts[2] = sizeof(preset3Default) / sizeof(preset3Default[0]);
    presetCounts[3] = sizeof(preset4Default) / sizeof(preset4Default[0]);

    copyPoseList(presetGroups[0], preset1Default, presetCounts[0]);
    copyPoseList(presetGroups[1], preset2Default, presetCounts[1]);
    copyPoseList(presetGroups[2], preset3Default, presetCounts[2]);
    copyPoseList(presetGroups[3], preset4Default, presetCounts[3]);
}

uint32_t calcTeachChecksum(const TeachStoreBlob &blob)
{
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(&blob);
    const size_t length = sizeof(TeachStoreBlob) - sizeof(blob.checksum);
    uint32_t hash = 2166136261u;

    for (size_t i = 0; i < length; ++i)
    {
        hash ^= ptr[i];
        hash *= 16777619u;
    }
    return hash;
}

void updateFlashInfoLabel()
{
    flashInfoText = "Flash: " + String(teachCount) + "个位姿";
}

bool saveTeachToFlash()
{
    TeachStoreBlob blob = {};
    blob.magic = kTeachMagic;
    blob.version = kTeachVersion;
    blob.count = teachCount;

    for (int i = 0; i < kTeachMaxPose; ++i)
    {
        blob.poses[i] = teachGroup[i];
    }

    blob.checksum = calcTeachChecksum(blob);
    size_t written = preferences.putBytes("teach", &blob, sizeof(blob));
    if (written != sizeof(blob))
    {
        return false;
    }

    updateFlashInfoLabel();
    return true;
}

bool loadTeachFromFlash()
{
    size_t length = preferences.getBytesLength("teach");
    if (length != sizeof(TeachStoreBlob))
    {
        return false;
    }

    TeachStoreBlob blob = {};
    size_t readSize = preferences.getBytes("teach", &blob, sizeof(blob));
    if (readSize != sizeof(blob))
    {
        return false;
    }

    if (blob.magic != kTeachMagic || blob.version != kTeachVersion || blob.count > kTeachMaxPose)
    {
        return false;
    }

    uint32_t check = calcTeachChecksum(blob);
    if (check != blob.checksum)
    {
        return false;
    }

    teachCount = blob.count;
    for (int i = 0; i < kTeachMaxPose; ++i)
    {
        teachGroup[i] = blob.poses[i];
    }

    updateTeachCountLabel();
    updateFlashInfoLabel();
    return true;
}

uint32_t calcPresetChecksum(const PresetStoreBlob &blob)
{
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(&blob);
    const size_t length = sizeof(PresetStoreBlob) - sizeof(blob.checksum);
    uint32_t hash = 2166136261u;

    for (size_t i = 0; i < length; ++i)
    {
        hash ^= ptr[i];
        hash *= 16777619u;
    }
    return hash;
}

bool savePresetsToFlash()
{
    PresetStoreBlob blob = {};
    blob.magic = kPresetMagic;
    blob.version = kPresetVersion;

    for (int group = 0; group < 4; ++group)
    {
        blob.counts[group] = presetCounts[group];
        for (int i = 0; i < kTeachMaxPose; ++i)
        {
            blob.poses[group][i] = presetGroups[group][i];
        }
    }

    blob.checksum = calcPresetChecksum(blob);
    size_t written = preferences.putBytes("preset", &blob, sizeof(blob));
    return written == sizeof(blob);
}

bool loadPresetsFromFlash()
{
    size_t length = preferences.getBytesLength("preset");
    if (length != sizeof(PresetStoreBlob))
    {
        return false;
    }

    PresetStoreBlob blob = {};
    size_t readSize = preferences.getBytes("preset", &blob, sizeof(blob));
    if (readSize != sizeof(blob))
    {
        return false;
    }

    if (blob.magic != kPresetMagic || blob.version != kPresetVersion)
    {
        return false;
    }

    for (int group = 0; group < 4; ++group)
    {
        if (blob.counts[group] > kTeachMaxPose)
        {
            return false;
        }
    }

    uint32_t check = calcPresetChecksum(blob);
    if (check != blob.checksum)
    {
        return false;
    }

    for (int group = 0; group < 4; ++group)
    {
        presetCounts[group] = blob.counts[group];
        for (int i = 0; i < kTeachMaxPose; ++i)
        {
            presetGroups[group][i] = blob.poses[group][i];
        }
    }

    return true;
}

void setStatus(const String &text)
{
    Serial.println(text);
    currentStatus = text;
}

void updateTeachCountLabel()
{
}

void sendPose(const Pose &pose)
{
    for (int index = 0; index < kJointCount; ++index)
    {
        const int motorId = kJointStartId + index;
        run(motorId, kDefaultSpeed, pose.joint[index]);
        delay(kPoseSendGapMs);
    }
}

void stopAllJoints()
{
    seq.running = false;
    for (int motorId = 1; motorId <= kGripperId; ++motorId)
    {
        stop(motorId);
        delay(5);
    }
}

bool readJointAngles(int outAngles[kJointCount])
{
    for (int i = 0; i < kJointCount; ++i)
    {
        float raw = read_Mc_angle(kJointStartId + i);
        if (raw < -10000.0f)
        {
            return false;
        }
        outAngles[i] = (int)lroundf(raw);
    }
    return true;
}

void updateJointAnglesForWeb(const int angles[kJointCount])
{
    for (int i = 0; i < kJointCount; ++i)
    {
        currentAngles[i] = angles[i];
    }
}

void refreshJointDisplay()
{
    if (jointReadState == JointReadState::IDLE)
    {
        jointReadIndex = 0;
        jointReadState = JointReadState::READING_JOINTS;
        can_begin_request(kJointStartId, 0x92, 0x240 + kJointStartId);
    }
}

void processJointRead()
{
    if (jointReadState == JointReadState::READING_JOINTS)
    {
        float result;
        if (can_poll_response(&result))
        {
            if (result < -10000.0f)
            {
                Serial.print("joint");
                Serial.print(jointReadIndex);
                Serial.println(" timeout");
            }
            else
            {
                currentAngles[jointReadIndex] = result;
                Serial.print("joint");
                Serial.print(jointReadIndex);
                Serial.print(" angle:");
                Serial.println(currentAngles[jointReadIndex]);
            }
            jointReadIndex++;
            if (jointReadIndex >= kJointCount)
            {
                jointReadState = JointReadState::READING_CLAW;
                can_begin_request(kGripperId, 0x00, 0x240 + kGripperId);
            }
            else
            {
                can_begin_request(kJointStartId + jointReadIndex, 0x92, 0x240 + kJointStartId + jointReadIndex);
            }
        }
    }
    else if (jointReadState == JointReadState::READING_CLAW)
    {
        float result;
        if (can_poll_response(&result))
        {
            if (result > -10000.0f)
            {
                currentAngles[6] = result;
            }
            jointReadState = JointReadState::IDLE;
            currentStatus = "关节位置已更新";
        }
    }
}

void webUpdateStatus(const String &status)
{
    currentStatus = status;
}

void startSequence(const Pose *poses, uint8_t count, const String &name)
{
    if (poses == nullptr || count == 0)
    {
        setStatus("动作组为空");
        return;
    }

    seq.poses = poses;
    seq.count = count;
    seq.index = 0;
    seq.running = true;
    seq.nextStepMs = 0;
    seq.name = name;
    setStatus(String("开始执行: ") + name);
}

void processSequence()
{
    if (!seq.running)
    {
        return;
    }

    unsigned long now = millis();
    if (now < seq.nextStepMs)
    {
        return;
    }

    if (seq.index >= seq.count)
    {
        seq.running = false;
        setStatus(String("执行完成: ") + seq.name);
        return;
    }

    const Pose &pose = seq.poses[seq.index];
    sendPose(pose);
    seq.nextStepMs = now + pose.holdMs;
    seq.index++;
}

void doSetAllZero()
{
    stopAllJoints();
    for (int motorId = 1; motorId <= kGripperId; ++motorId)
    {
        set_zero(motorId);
        delay(120);
        re_set(motorId);
        delay(180);
    }
    setStatus("已设置当前位姿为全关节零点");
}

void doRecordTeachPose()
{
    if (teachCount >= kTeachMaxPose)
    {
        setStatus("示教组已满(10个位姿)");
        return;
    }

    int current[kJointCount] = {0};
    if (!readJointAngles(current))
    {
        setStatus("记录失败: 读取关节失败");
        return;
    }

    for (int i = 0; i < kJointCount; ++i)
    {
        teachGroup[teachCount].joint[i] = current[i];
    }
    teachGroup[teachCount].holdMs = 3000;
    teachCount++;

    updateJointAnglesForWeb(current);
    setStatus(String("示教记录成功: 第") + String(teachCount) + String("个位姿"));
}

void doRunTeach()
{
    if (teachCount == 0)
    {
        setStatus("示教组为空, 请先记录位姿");
        return;
    }
    startSequence(teachGroup, teachCount, "示教动作组");
}

void doSaveTeachToFlash()
{
    if (saveTeachToFlash())
    {
        currentStatus = "示教组已保存到Flash";
    }
    else
    {
        currentStatus = "保存失败: Flash写入异常";
    }
}

void doEraseFlash()
{
    preferences.remove("teach");
    currentStatus = "Flash示教组已删除";
}

void saveCurrentPoseToPreset(int index)
{
    saveTeachToPreset(index);
}

void runPreset(int index)
{
    if (index >= 4 || presetCounts[index] == 0)
    {
        currentStatus = "预设" + String(index + 1) + "为空";
        return;
    }
    startSequence(presetGroups[index], presetCounts[index], "预设动作" + String(index + 1));
}

void saveTeachToPreset(uint8_t presetIndex)
{
    if (presetIndex >= 4)
    {
        setStatus("保存失败: 预设编号非法");
        return;
    }

    if (teachCount == 0)
    {
        setStatus("保存失败: 示教组为空");
        return;
    }

    presetCounts[presetIndex] = teachCount;
    copyPoseList(presetGroups[presetIndex], teachGroup, teachCount);

    bool ok = savePresetsToFlash();
    if (ok)
    {
        setStatus(String("已保存示教组到预设") + String(presetIndex + 1));
    }
    else
    {
        setStatus(String("已写入预设") + String(presetIndex + 1) + String(", 但Flash保存失败"));
    }
}

void doEmergencyStop()
{
    stopAllJoints();
    setStatus("急停触发: 已停止机械臂动作");
}

void doClawOpen()
{
    run(kGripperId, 80, 90);
    setStatus("夹爪张开");
}

void doClawClose()
{
    run(kGripperId, 80, 0);
    setStatus("夹爪闭合");
}

void setupWifi()
{
    Serial.println("\nStart AP mode");
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_AP);
    delay(200);

    uint32_t chipid = 0;
    for (int i = 0; i < 17; i += 8)
    {
        chipid |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }

    char apSsid[26];
    snprintf(apSsid, sizeof(apSsid), "ARM-ESPUI-%08X", chipid);
    
    bool apSuccess = WiFi.softAP(apSsid, apSsid);
    Serial.print("softAP result: ");
    Serial.println(apSuccess ? "SUCCESS" : "FAILED");
    
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    Serial.print("AP SSID: ");
    Serial.println(apSsid);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("AP status: ");
    Serial.println(WiFi.status());
}

void setupWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", MAIN_PAGE);
    });

    server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request) {
        String action = "";
        if (request->hasParam("action")) {
            action = request->getParam("action")->value();
        }

        if (action == "readJoints") {
            refreshJointDisplay();
        }
        else if (action == "record") {
            doRecordTeachPose();
        }
        else if (action == "runTeach") {
            doRunTeach();
        }
        else if (action == "clearTeach") {
            teachCount = 0;
            updateTeachCountLabel();
            currentStatus = "示教组已清空(内存)";
        }
        else if (action == "saveFlash") {
            doSaveTeachToFlash();
        }
        else if (action == "loadFlash") {
            if (loadTeachFromFlash()) {
                currentStatus = "加载示教组成功: " + String(teachCount) + "个位姿";
            } else {
                currentStatus = "加载示教组失败";
            }
        }
        else if (action == "deleteFlash") {
            doEraseFlash();
            currentStatus = "Flash示教组已删除";
        }
        else if (action == "clawOpen") {
            doClawOpen();
            currentStatus = "夹爪已张开";
        }
        else if (action == "clawClose") {
            doClawClose();
            currentStatus = "夹爪已闭合";
        }
        else if (action == "savePreset1") {
            saveCurrentPoseToPreset(0);
        }
        else if (action == "savePreset2") {
            saveCurrentPoseToPreset(1);
        }
        else if (action == "savePreset3") {
            saveCurrentPoseToPreset(2);
        }
        else if (action == "savePreset4") {
            saveCurrentPoseToPreset(3);
        }
        else if (action == "preset1") {
            runPreset(0);
        }
        else if (action == "preset2") {
            runPreset(1);
        }
        else if (action == "preset3") {
            runPreset(2);
        }
        else if (action == "preset4") {
            runPreset(3);
        }
        else if (action == "emergency") {
            doEmergencyStop();
            currentStatus = "紧急停止!";
        }
        else if (action == "setZero") {
            doSetAllZero();
            currentStatus = "全关节已置零";
        }

        request->send(200, "text/plain", "OK");
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"status\":\"" + currentStatus + "\",";
        json += "\"angles\":[";
        for (int i = 0; i < 7; i++) {
            json += String(currentAngles[i], 2);
            if (i < 6) json += ",";
        }
        json += "],";
        json += "\"teachCount\":" + String(teachCount) + ",";
        json += "\"flashInfo\":\"" + flashInfoText + "\"";
        json += "}";
        request->send(200, "application/json", json);
    });

    server.begin();
    Serial.println("Web server started");
}

void setup()
{
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(RELAY_B_PIN, OUTPUT);
    digitalWrite(RELAY_B_PIN, HIGH);

    Serial.begin(115200);
    delay(200);

    if (!rc_init())
    {
        Serial.println("CAN init failed");
    }

    setupWifi();
    setupWebServer();

    preferences.begin("armteach", false);
    initDefaultPresets();

    if (loadPresetsFromFlash())
    {
        Serial.println("Loaded preset groups from flash");
    }

    if (loadTeachFromFlash())
    {
        currentStatus = "开机自动恢复示教组: " + String(teachCount) + "个位姿";
    }

    esp_task_wdt_init(30, true);
    esp_task_wdt_add(NULL);
}

void loop()
{
    esp_task_wdt_reset();
    processJointRead();
    processSequence();
    delay(5);
}
