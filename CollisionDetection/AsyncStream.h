/*
    Библиотека для асинхронного чтения объектов Stream (Serial итд)
    Документация: 
    GitHub: https://github.com/GyverLibs/AsyncStream
    Возможности:
    - Неблокирующее чтение в свой буфер
    - Указание символа терминатора
    
    AlexGyver, alex@alexgyver.ru
    https://alexgyver.ru/
    MIT License

    Версии:
    v1.0 - релиз
    v1.1 - исправлен баг
*/

#ifndef _AsyncStream_h
#define _AsyncStream_h
#include <Arduino.h>

template < uint16_t SIZE >
class AsyncStream 
{
public:
    AsyncStream(Stream* port, uint8_t len = 4, uint16_t tout = 5) 
    {
        _port = port;
        _tout = tout;
        _len = len;
    }
    
    // установить таймаут
    void setTimeout(uint16_t tout) 
    {
        _tout = tout;
    }

    void setLen(uint8_t len)
    {
        _len = len;
    }
    
    // данные приняты
    bool available() 
    {
      while (_port -> available()) 
      {
        if (!_parseF) 
        {
          _parseF = true;
          _count = 0;
          _tmr = millis();
        }
        char ch = _port -> read();
        if ((_count + 1) == _len)
        {
          buf[_count] = ch;
          _parseF = false;
          return true;
        }
        else if (_count < SIZE - 1) 
        {
          buf[_count++] = ch;
        }
        _tmr = millis();
      }
      if (_parseF && millis() - _tmr >= _tout) 
      {
        // Serial.println("time");
        _parseF = false;
        buf[_count] = '\0';
        return true;
      }
      return false;
    }
    
    // доступ к буферу
    char buf[SIZE];

private:
    Stream* _port;
    uint8_t _len;
    uint16_t _tout, _count = 0;
    uint32_t _tmr = 0;
    bool _parseF = false;
};

#endif
