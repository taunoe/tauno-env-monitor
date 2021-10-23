/**
 * File: secrets.h
 * Started 29.10.2020
 * Edited 23.10.2021
 * Copyright 2021 Tauno Erik
 * 
 * Wifi and ThingSpeak passwords
 */

#ifndef TAUNO_SECRETS_H_
#define TAUNO_SECRETS_H_

namespace Secret {
  const char * ssd1 = "wifi name";
  const char * pass1 = "wifi password";
  const char * ssd2 = "wifi name";
  const char * pass2 = "wifi password";
  // Thingspeak
  const int id = 012345;
  const char * key = "MYSECRETKEY";
}  // namespace Secret

#endif  // TAUNO_SECRETS_H_
