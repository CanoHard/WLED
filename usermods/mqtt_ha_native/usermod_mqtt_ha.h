#pragma once

#include "wled.h"

//This is an empty v2 usermod template. Please see the file usermod_v2_example.h in the EXAMPLE_v2 usermod folder for documentation on the functions you can use!

String p[50];

class Usermod_mqtt_ha : public Usermod
{
private:
  const char *availability_topic = "test/av";
  const char *command_topic = "test/wled";
  const char *state_topic = "test/wled/s";
  bool mqttInitialized;
  bool on = false;
  bool init = false;
  char *payload_o;
  int brig = 0;
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
    String topic_ = topic;
    String msj;
    msj = "";
    for (int i = 0; i < len; i++)
    { //Le el msj
      msj = msj + (char)payload[i];
    }
    String Topic = topic;
    if (topic_ == command_topic)
    {
      //char payload[192];

      StaticJsonDocument<192> doc2;
      StaticJsonDocument<128> doc;

      DeserializationError error = deserializeJson(doc, msj);

      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      String state = doc["state"]; 

      if (state == "ON" && !on)
      {
        toggleOnOff();
        on = true;
      }
      else if (state == "OFF" && on)
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
      const char *effect = doc["effect"]; // "[FX=19] Dissolve Rnd"
      colorUpdated(NOTIFIER_CALL_MODE_BUTTON);

      char sendmsj[180];
      size_t payload_size = serializeJson(doc, sendmsj);
      if (WLED_MQTT_CONNECTED)
      {
        Serial.println(sendmsj);
        mqtt->publish(state_topic, 0, false, sendmsj, payload_size); //Wled state
      }
    }
  }

public:
  void setup()
  {
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

      int effect_p = root["seg"]["sx"];
      doc["color_mode"] = "color_temp";
      doc["color_temp"] = effect_p;
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
    if (root["seg"].containsKey("sx"))
    {
      int effect_p = root["seg"]["sx"];
      doc["color_mode"] = "color_temp";
      doc["color_temp"] = effect_p;
    }

    //doc["effect"] = "[FX=10] Scan";

    size_t payload_size = serializeJson(doc, payload);
    //Serial.println(payload);
    if (WLED_MQTT_CONNECTED)
    {
      mqtt->publish(state_topic, 0, false, payload, payload_size); //Wled state
    }
    else
    {
      payload_o = payload; //Last state
    }
  }
};