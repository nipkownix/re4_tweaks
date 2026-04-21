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
		};

		struct SndQueuedStop
		{
			uint32_t sndCallHandle;
			int sndStopParam;
			double expiryTick;
			bool active;
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

			// Deferred stops live separately from the dedup buffers so buffer rotation can't eat any pending stops.
			static constexpr size_t MAX_QUEUED_STOPS = 256;
			static inline SndQueuedStop queuedStops[MAX_QUEUED_STOPS];
			static inline size_t queuedStopCount = 0;

			static bool QueueStop(uint32_t sndCallHandle, int time, double expiryTick)
			{
				// Check if already queued
				for (size_t i = 0; i < queuedStopCount; i++)
				{
					if (queuedStops[i].active && queuedStops[i].sndCallHandle == sndCallHandle)
						return true;
				}

				// Try to reuse an inactive slot
				for (size_t i = 0; i < queuedStopCount; i++)
				{
					if (!queuedStops[i].active)
					{
						queuedStops[i] = { sndCallHandle, time, expiryTick, true };
						return true;
					}
				}

				if (queuedStopCount < MAX_QUEUED_STOPS)
				{
					queuedStops[queuedStopCount++] = { sndCallHandle, time, expiryTick, true };
					return true;
				}

				// No room in the queue?
				return false;
			}

			static bool CancelQueuedStop(uint32_t sndCallHandle)
			{
				for (size_t i = 0; i < queuedStopCount; i++)
				{
					if (queuedStops[i].active && queuedStops[i].sndCallHandle == sndCallHandle)
					{
						queuedStops[i].active = false;
						return true;
					}
				}

				return false;
			}

			// Called once per frame from `Framelimiter_Hook`
			static void Tick(double now, std::function<void(uint32_t id, int time)> stopCb)
			{
				// Process any queued stops
				for (size_t i = 0; i < queuedStopCount; i++)
				{
					if (queuedStops[i].active && now >= queuedStops[i].expiryTick)
					{
						stopCb(queuedStops[i].sndCallHandle, queuedStops[i].sndStopParam);

						// Mark the dedup entry so SndCall_Hook won't return this stale handle
						SndKey* key = buffers[current].find(queuedStops[i].sndCallHandle);
						if (!key)
							key = buffers[prev()].find(queuedStops[i].sndCallHandle);
						if (key)
							key->isStopped = true;

						queuedStops[i].active = false;
					}
				}

				double elapsed = now - slice_start;
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