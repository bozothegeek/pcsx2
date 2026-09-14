// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "svnrev.h"

namespace BuildVersion
{
	const char* GitTag = "v2.8.2";
	bool GitTaggedCommit = 1;
	int GitTagHi = 2;
	int GitTagMid = 8;
	int GitTagLo = 2;
	const char* GitRev = "v2.8.2";
	const char* GitHash = "fd9d310ccbb6b8b62c976da8886a3c8fd3a10ff3";
	const char* GitDate = GIT_DATE;
} // namespace BuildVersion
