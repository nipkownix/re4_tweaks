#pragma once
#include <functional>

namespace re4t
{
	namespace AudioTweaks
	{
		void UpdateVolume();

		struct SndKey
		{
			void* ret_addr;
			uint16_t blk;
			uint16_t call_no;
			cModel* pMod;
			uint32_t flag;
			uint32_t sndCallHandle;
			double tick;
			bool isStopped;
			double expiryTick;
			int sndStopParam;
		};

		struct SndSlice
		{
			static constexpr double SLICE_DURATION = 1000.0 / 30.0;
			static constexpr size_t MAX_ENTRIES = 256;
			SndKey entries[MAX_ENTRIES];
			size_t count = 0;

			void clear() { count = 0; }

			SndKey* find(void* ret, uint16_t blk, uint16_t call_no, uint32_t flag, cModel* pMod)
			{
				for (size_t i = 0; i < count; i++)
				{
					if (entries[i].pMod == pMod &&
						entries[i].flag == flag &&
						entries[i].call_no == call_no &&
						entries[i].blk == blk &&
						entries[i].ret_addr == ret)
						return &entries[i];
				}
				return nullptr;
			}

			SndKey* find(uint32_t sndCallHandle)
			{
				for (size_t i = 0; i < count; i++)
				{
					if (entries[i].sndCallHandle == sndCallHandle)
						return &entries[i];
				}
				return nullptr;
			}

			void on_expired(double now, std::function<void(SndKey*)> expiredCb)
			{
				for (size_t i = 0; i < count; i++)
				{
					if (entries[i].expiryTick == 0)
						continue;
					if (now - entries[i].expiryTick >= SLICE_DURATION)
						expiredCb(&entries[i]);
				}
			}

			bool add(const SndKey& key)
			{
				if (count >= MAX_ENTRIES)
					return false;
				entries[count++] = key;
				return true;
			}
		};

		class SndDedup
		{
		public:

			// Two buffers rotated every ~33ms, duplicates are checked against both so sounds straddling a timeslice boundary are still caught.
			// Matches in the previous buffer are only accepted if within the 33ms dedup window.
			static inline SndSlice buffers[2];
			static inline int current = 0;
			static inline double slice_start = 0;

			// Called once per frame from `Framelimiter_Hook`
			static void Tick(double now, std::function<void(SndKey*)> expiredCb)
			{
				double elapsed = now - slice_start;

				buffers[0].on_expired(now, expiredCb);
				buffers[1].on_expired(now, expiredCb);

				if (elapsed >= SndSlice::SLICE_DURATION)
				{
					if (elapsed >= (SndSlice::SLICE_DURATION * 2))
					{
						buffers[0].clear();
						buffers[1].clear();
					}
					else
					{
						current ^= 1;
						buffers[current].clear();
					}

					slice_start = now;
				}
			}

			static int prev() { return current ^ 1; }
		};
	}
}