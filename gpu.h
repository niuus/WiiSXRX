/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2016  Pcsx Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses>.
 */

/*
 * q: Why bother with GPU stuff in a plugin-based emu core?
 * a: mostly because of busy bits, we have all the needed timing info
 *    that GPU plugin doesn't.
 */

#ifndef __GPU_H__
#define __GPU_H__

#define PSXGPU_LCF     (1<<31)
#define PSXGPU_nBUSY   (1<<26)
#define PSXGPU_ILACE   (1<<22)
#define PSXGPU_DHEIGHT (1<<19)

// both must be set for interlace to work
#define PSXGPU_ILACE_BITS (PSXGPU_ILACE | PSXGPU_DHEIGHT)

#define HW_GPU_STATUS psxHu32ref(0x1814)

// TODO: handle com too
#define PSXGPU_TIMING_BITS (PSXGPU_LCF | PSXGPU_nBUSY)

#define gpuSyncPluginSR() { \
	HW_GPU_STATUS &= PSXGPU_TIMING_BITS; \
	HW_GPU_STATUS |= GPU_readStatus() & ~PSXGPU_TIMING_BITS; \
}

#endif /* __GPU_H__ */
