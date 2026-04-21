#pragma once

namespace re4t
{
	namespace AudioTweaks
	{
		void UpdateVolume();

        struct SndEntry
        {
            // Dedup key
            void* ret_addr;
            uint16_t blk;
            uint16_t call_no;
            cModel* pMod;
            uint32_t flag;

            // Tracking
            uint32_t sndCallHandle;
            double startTick;

            // Deferred stop (pendingStopAt == 0 means none queued)
            double pendingStopAt;
            int pendingStopParam;

            bool isStopped;
        };

        class SndDedup
        {
        public:
            static constexpr double DEDUP_WINDOW = 1000.0 / 30.0; // ~33ms
            static constexpr size_t MAX_ENTRIES = 512;

            static inline SndEntry entries[MAX_ENTRIES];
            static inline size_t   count = 0;

            static SndEntry* find(void* ret, uint16_t blk, uint16_t call_no, uint32_t flag, cModel* pMod)
            {
                for (size_t i = count; i > 0; --i)
                {
                    SndEntry& e = entries[i - 1];
                    if (e.pMod == pMod && e.flag == flag && e.call_no == call_no &&
                        e.blk == blk && e.ret_addr == ret)
                        return &e;
                }
                return nullptr;
            }

            static SndEntry* find(uint32_t sndCallHandle)
            {
                for (size_t i = count; i > 0; --i)
                {
                    if (entries[i - 1].sndCallHandle == sndCallHandle)
                        return &entries[i - 1];
                }
                return nullptr;
            }

            static SndEntry* add(const SndEntry& entry)
            {
                if (count >= MAX_ENTRIES)
                    return nullptr;
                entries[count] = entry;
                return &entries[count++];
            }

            // Called once per frame from Framelimiter_Hook.
            // Fires any expired pending stops, then compacts out entries that are past
            // the dedup window and have no pending stop.
            template<typename StopFn>
            static void Tick(double now, StopFn&& stopCb)
            {
                size_t write = 0;
                for (size_t read = 0; read < count; ++read)
                {
                    SndEntry& e = entries[read];

                    if (e.pendingStopAt > 0 && now >= e.pendingStopAt)
                    {
                        stopCb(e.sndCallHandle, e.pendingStopParam);
                        e.pendingStopAt = 0;
                        e.isStopped = true;
                    }

                    const bool inWindow = (now - e.startTick) < DEDUP_WINDOW;
                    const bool hasPendingStop = (e.pendingStopAt > 0);

                    if (inWindow || hasPendingStop)
                    {
                        if (write != read)
                            entries[write] = e;
                        ++write;
                    }
                }
                count = write;
            }
        };
	}
}