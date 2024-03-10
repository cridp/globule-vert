#ifndef IOHC_RADIO_H
#define IOHC_RADIO_H

#include <memory>
#include <board-config.h>

#include <map>
#include <Delegate.h>
#include <iohcCryptoHelpers.h>
#if defined(SX1276)
        #include <SX1276Helpers.h>
#elif defined(CC1101)
        #include <CC1101Helpers.h>
#endif

#include <iohcPacket.h>
#if defined(ESP8266)
  #include <TickerUs.h>
#elif defined(HELTEC)  	
    #include <TickerUsESP32.h>
#endif

#define SM_GRANULARITY_US               130ULL  // Ticker function frequency in uS (100 minimum) 4 x 26µs = 104
#define SM_GRANULARITY_MS               1       // Ticker function frequency in uS
#define SM_PREAMBLE_RECOVERY_TIMEOUT_US 1378 // 12500   // SM_GRANULARITY_US * PREAMBLE_LSB //12500   // Maximum duration in uS of Preamble before reset of receiver
#define DEFAULT_SCAN_INTERVAL_US        13520   // Default uS between frequency changes

/*
    Singleton class to implement an IOHC Radio abstraction layer for controllers.
    Implements all needed functionalities to receive and send packets from/to the air, masking complexities related to frequency hopping
    IOHC timings, async sending and receiving through callbacks, ...
*/
namespace IOHC {
    using IohcPacketDelegate = Delegate<bool(iohcPacket *iohc)>;

    class iohcRadio  {
        public:
            static iohcRadio *getInstance();
            virtual ~iohcRadio() = default;
            void start(uint8_t num_freqs, uint32_t *scan_freqs, uint32_t scanTimeUs, IohcPacketDelegate rxCallback, IohcPacketDelegate txCallback);
            void send(std::vector<iohcPacket*>&iohcTx);

        private:
            iohcRadio();
            bool receive(bool stats);
            bool sent(iohcPacket *packet);

            static iohcRadio *_iohcRadio;
            volatile static bool _g_preamble;
            volatile static bool _g_payload;
            static uint8_t _flags[2];
            volatile static unsigned long _g_payload_millis;
            
            volatile static bool f_lock;
            volatile static bool send_lock;
            volatile static bool txMode;

            volatile uint32_t tickCounter = 0;
            volatile uint32_t preCounter = 0;
            volatile uint8_t txCounter = 0;

            uint8_t num_freqs = 0;
            uint32_t *scan_freqs{};
            uint32_t scanTimeUs{};
            uint8_t currentFreqIdx = 0;


        #if defined(ESP8266)
            Timers::TickerUs TickTimer;
            Timers::TickerUs Sender;
//            Timers::TickerUs FreqScanner;
        #elif defined(HELTEC)
            TimersUS::TickerUsESP32 TickTimer;
            TimersUS::TickerUsESP32 Sender;
        #endif
            iohcPacket *iohc{};
            iohcPacket *delayed{};
            
            IohcPacketDelegate rxCB = nullptr;
            IohcPacketDelegate txCB = nullptr;
            std::vector<iohcPacket*> packets2send{};
        protected:
            static void IRAM_ATTR i_preamble();
            static void IRAM_ATTR i_payload();
            static void IRAM_ATTR tickerCounter(iohcRadio *radio);

            static void packetSender(iohcRadio *radio);

        #if defined(CC1101)
            uint8_t lenghtFrame=0;
        #endif
    };
}

#endif