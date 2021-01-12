/*
 *	Copyright (C) 2020 PCSX2 Dev Team
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "MultiISA.h"
#include "GSUtil.h"

static VectorISA getCurrentISA()
{
	if (g_cpu.has(Xbyak::util::Cpu::tAVX2))
		return VectorISA::AVX2;
	else if (g_cpu.has(Xbyak::util::Cpu::tAVX))
		return VectorISA::AVX;
	else if (g_cpu.has(Xbyak::util::Cpu::tSSE41))
		return VectorISA::SSE4;
	else
		return VectorISA::SSE2;
}

VectorISA currentISA = getCurrentISA();

bool s_nativeres;
CRC::Region g_crc_region = CRC::NoRegion;
int GSStateISAShared::s_n = 0;
