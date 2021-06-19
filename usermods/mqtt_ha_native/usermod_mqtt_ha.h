#pragma once

#include "wled.h"

//This usermod makes wled compatible with the native mqtt light implementation in home assistant
//Made by Pablo Cano

class Usermod_mqtt_ha : public Usermod
{
private:
  const char *availability_topic = "test/av";
  const char *command_topic = "test/wled";
  const char *state_topic = "test/wled/s";
  bool mqttInitialized;
  bool on = false;
  bool init = false;
  char payload_o[192];
  String modes[MODE_COUNT];
  void mqttInit()
  {
    if (!mqtt)
      return;
    mqtt->onMessage(
        std::bind(&Usermod_mqtt_ha::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
                  std::placeholders::_5, std::placeholders::_6));
    mqtt->onConnect(std::bind(&Usermod_mqtt_ha::onMqttConnect, this, std::placeholders::_1));
    mqtt->setWill(availability_topic, 0, true, "offline"); //Will topic
    mqttInitialized = true;
  }
  void onMqttConnect(bool sessionPresent)
  {
    mqtt->subscribe(command_topic, 0);                    //Subscribe to receive commands
    mqtt->publish(availability_topic, 0, true, "online"); //Will topic
    mqtt->publish(state_topic, 0, false, payload_o);      //Wled state
  }

  void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
  {
    if (bri == 0)
    {
      on = false;
    }
    else
    {
      on = true;
    }

    if (strcmp(topic, command_topic) == 0)
    {
      //char payload[192];

      StaticJsonDocument<192> doc2;
      StaticJsonDocument<128> doc;

      DeserializationError error = deserializeJson(doc, payload);

      if (error)
      {
        // Serial.print(F("deserializeJson() failed: "));
        // Serial.println(error.f_str());
        return;
      }

      const char *state = doc["state"];

      if (strcmp(state, "ON") == 0 && !on)
      {
        toggleOnOff();
        on = true;
      }
      else if (strcmp(state, "OFF") == 0 && on)
      {
        toggleOnOff();
        on = false;
      }

      if (doc.containsKey("color"))
      {
        JsonObject color = doc["color"];
        int r = color["r"]; // 191
        int g = color["g"]; // 191
        int b = color["b"]; // 255
        col[0] = r;
        col[1] = g;
        col[2] = b;
        doc["color_mode"] = "rgb";
        JsonObject color2 = doc2.createNestedObject("color");
        color2["r"] = r;
        color2["g"] = g;
        color2["b"] = b;
      }
      if (doc.containsKey("brightness"))
      {
        bri = doc["brightness"];

        doc2["brightness"] = bri;
      }
      if (doc.containsKey("color_temp"))
      {
        effectSpeed = doc["color_temp"];
        doc["color_mode"] = "color_temp";
        doc2["color_temp"] = effectSpeed;
      }
      if (doc.containsKey("effect"))
      {
        const char *effect = doc["effect"];
        effectCurrent = searchmode(effect);
      }
      colorUpdated(NOTIFIER_CALL_MODE_BUTTON);

      char sendmsj[180];
      size_t payload_size = serializeJson(doc, sendmsj);
      if (WLED_MQTT_CONNECTED)
      {
        mqtt->publish(state_topic, 0, false, sendmsj, payload_size); //Wled state
      }
    }
  }

  void getmodes() //Parse JSON_mode_names to String[]
  {
    String modeseffe = JSON_mode_names;
    int curremode = 0;
    bool stringproc = false;
    int firstindex = 0;
    for (int i = 0; i < sizeof(JSON_mode_names); i++)
    {
      if (JSON_mode_names[i] == '"')
      {
        if (!stringproc)
        {
          firstindex = i + 1;
          stringproc = true;
        }
        else
        {
          modes[curremode] = modeseffe.substring(firstindex, i);
          stringproc = false;
          curremode++;
        }
      }
    }
  }

  int searchmode(const char *mode_name)
  {
    for (int i = 0; i < MODE_COUNT; i++)
    {
      if (strcmp(mode_name, modes[i].c_str()) == 0)
      {
        return i;
      }
    }
  }

public:
  void setup()
  {
    getmodes();
  }

  void loop()
  {
    if (!mqttInitialized)
    {
      mqttInit();
      return; // Try again in next loop iteration
    }
  }

  void addToJsonState(JsonObject &root)
  {
  }

  /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
  void readFromJsonState(JsonObject &root)
  {
    char payload[192];

    StaticJsonDocument<192> doc;

    if (bri == 0)
    {
      on = false;
    }
    else
    {
      on = true;
    }

    if (root.containsKey("on"))
    {
      on = root["on"];
    }
    if (root.containsKey("bri"))
    {
      doc["brightness"] = root["bri"];
      on = true;
    }
    if (on)
    {
      doc["state"] = "ON";
    }
    else
    {
      doc["state"] = "OFF";
    }

    if (!init)
    {
      JsonArray seg = root["seg"];

      int effect_p = seg[0]["sx"];
      doc["color_mode"] = "color_temp";
      doc["color_temp"] = effect_p;

      doc["color_mode"] = "rgb";

      int r, g, b;
      JsonArray seg_col = seg[0]["col"];

      if (seg_col[1].size() > 0)
      {
        JsonArray seg_col_1 = seg_col[1];
        r = seg_col_1[0];
        g = seg_col_1[1];
        b = seg_col_1[2];
      }

      if (seg_col[2].size() > 0)
      {
        JsonArray seg_col_2 = seg_col[2];
        r = seg_col_2[0];
        g = seg_col_2[1];
        b = seg_col_2[2];
      }

      if (seg_col[0].size() > 0)
      {
        JsonArray seg_col_0 = seg_col[0];
        r = seg_col_0[0];
        g = seg_col_0[1];
        b = seg_col_0[2];
      }
      JsonObject color = doc.createNestedObject("color");
      color["r"] = r;
      color["g"] = g;
      color["b"] = b;

      int nummode = seg[0]["fx"];
      doc["effect"] = modes[nummode];
      init = true;
    }

    if (root["seg"].containsKey("col"))
    {
      doc["color_mode"] = "rgb";
      int r, g, b;
      JsonArray seg_col = root["seg"]["col"];

      if (seg_col[1].size() > 0)
      {
        JsonArray seg_col_1 = seg_col[1];
        r = seg_col_1[0];
        g = seg_col_1[1];
        b = seg_col_1[2];
      }

      if (seg_col[2].size() > 0)
      {
        JsonArray seg_col_2 = seg_col[2];
        r = seg_col_2[0];
        g = seg_col_2[1];
        b = seg_col_2[2];
      }
      if (seg_col[0].size() > 0)
      {
        JsonArray seg_col_0 = seg_col[0];
        r = seg_col_0[0];
        g = seg_col_0[1];
        b = seg_col_0[2];
      }
      JsonObject color = doc.createNestedObject("color");
      color["r"] = r;
      color["g"] = g;
      color["b"] = b;
    }

    if (root["seg"].containsKey("fx"))
    {
      int nummode = root["seg"]["fx"];
      doc["effect"] = modes[nummode];
    }

    size_t payload_size = serializeJson(doc, payload);
    if (WLED_MQTT_CONNECTED)
    {
      mqtt->publish(state_topic, 0, false, payload, payload_size); //Wled state
    }
    else
    {

      strcpy(payload_o, payload);
    }
  }
};