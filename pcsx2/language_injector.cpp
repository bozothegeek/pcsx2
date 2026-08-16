/*
* Language Injector by SeventySixx
* According to the provided BIOS path and language option, injects the relative 
* language byte in the nvm BIOS file. If a BIOS is not supported, or if the requested 
* language is already set, it exits without doing nothing. 
*/

#include "language_injector.h"
#include <fstream>
#include <string>
#include <cctype>

// Forward declarations of core structures
struct StereoOut16;
struct StereoOut32;
struct StereoOutFloat;
struct V_Core;

// SPU2 internal headers (adjust relative paths if your .cpp file is outside the SPU2 folder)
#include "SPU2/defs.h"   // Core SPU2 definitions and constants
#include "SPU2/regs.h"   // SPU2 hardware register definitions
#include "SPU2/Debug.h"  // SPU2 debugging utilities and logging
#include "SPU2/spu2.h"
#include "SPU2/interpolate_table.h"
#include "SPU2/Dma.h"

#include "ps2/BiosTools.h"

#include "common/Console.h"
#include "common/FileSystem.h"

static const char* BIOS_SCPH30003		    = "Europev01.20(02/09/2000)Console";
static const char* BIOS_SCPH30004R			= "Europev01.60(04/10/2001)Console";  //BIOS_SCPH30004R (BIOS_SCPH30004 not supported)
static const char* BIOS_SCPH39004			= "Europev01.60(19/03/2002)Console";
static const char* BIOS_SCPH50003			= "Europev02.00(04/11/2004)Console";
static const char* BIOS_SCPH50004			= "Europev01.90(23/06/2003)Console";
static const char* BIOS_SCPH70004			= "Europev02.00(14/06/2004)Console";
static const char* BIOS_SCPH75004			= "Europev02.20(20/06/2005)Console";
static const char* BIOS_SCPH77004			= "Europev02.20(10/02/2006)Console";
static const char* BIOS_EU_230				= "Europev02.30(20/02/2008)Console20080220-175343";

static const char* BIOS_SCPH39001			= "USAv01.60(07/02/2002)Console";
static const char* BIOS_SCPH39001V2			= "USAv01.60(19/03/2002)Console";
static const char* BIOS_SCPH70012			= "USAv02.00(14/06/2004)Console";
static const char* BIOS_SCPH77001			= "USAv02.20(10/02/2006)Console";
static const char* BIOS_SCPH90001			= "USAv02.30(20/02/2008)Console";
static const char* BIOS_USA_230				= "USAv02.30(20/02/2008)Console20080220-175343";

static const char* BIOS_JAP_230				= "Japanv02.30(20/02/2008)Console20080220-175343";

static const char* BIOS_ASIA_230			= "Asiav02.30(20/02/2008)Console20080220-175343";

static const uint8_t ADDRESS_SCPH30003	= 0x31;
static const uint8_t ADDRESS_SCPH30004R = 0x31;
static const uint8_t ADDRESS_SCPH39004	= 0x31;
static const uint8_t ADDRESS_SCPH50003	= 0x2c;
static const uint8_t ADDRESS_SCPH50004	= 0x2c;
static const uint8_t ADDRESS_SCPH70004	= 0x2c;
static const uint8_t ADDRESS_SCPH75004	= 0x2c;
static const uint8_t ADDRESS_SCPH77004	= 0x2c;
static const uint8_t ADDRESS_EU_230		= 0x2c;

static const uint8_t ADDRESS_SCPH39001	= 0x31;
static const uint8_t ADDRESS_SCPH39001V2= 0x31;
static const uint8_t ADDRESS_SCPH70012	= 0x2c;
static const uint8_t ADDRESS_SCPH77001	= 0x2c;
static const uint8_t ADDRESS_SCPH90001	= 0x2c;
static const uint8_t ADDRESS_USA_230	= 0x2c;

static const uint8_t ADDRESS_JAP_230	= 0x2c;

static const uint8_t ADDRESS_ASIA_230	= 0x2c;

static const char* LANG_JAPANESE	= "Japanese";
static const char* LANG_ENGLISH		= "English";
static const char* LANG_FRENCH		= "French";
static const char* LANG_SPANISH		= "Spanish";
static const char* LANG_GERMAN		= "German";
static const char* LANG_ITALIAN		= "Italian";
static const char* LANG_DUTCH		= "Dutch";
static const char* LANG_PORTUGUESE	= "Portuguese";
static const char* LANG_CHINESE		= "Chinese";


static const uint8_t BYTE_LANG_JAP = 0x20;
static const uint8_t BYTE_LANG_ENG = 0x21;
static const uint8_t BYTE_LANG_FRA = 0x22;
static const uint8_t BYTE_LANG_SPA = 0x23;
static const uint8_t BYTE_LANG_GER = 0x24;
static const uint8_t BYTE_LANG_ITA = 0x25;
static const uint8_t BYTE_LANG_DUT = 0x26;
static const uint8_t BYTE_LANG_POR = 0x27;
static const uint8_t BYTE_LANG_TCH = 0x2A;

static const int NUM_BIOS_LANG_ENTRIES = 109;


const bios_lang bios_language[NUM_BIOS_LANG_ENTRIES] = {
	// europe
	{ BIOS_SCPH30003, LANG_ENGLISH,		ADDRESS_SCPH30003, BYTE_LANG_ENG },
	{ BIOS_SCPH30003, LANG_FRENCH,		ADDRESS_SCPH30003, BYTE_LANG_FRA },
	{ BIOS_SCPH30003, LANG_SPANISH,		ADDRESS_SCPH30003, BYTE_LANG_SPA },
	{ BIOS_SCPH30003, LANG_GERMAN,		ADDRESS_SCPH30003, BYTE_LANG_GER },
	{ BIOS_SCPH30003, LANG_ITALIAN,		ADDRESS_SCPH30003, BYTE_LANG_ITA },
	{ BIOS_SCPH30003, LANG_DUTCH,		ADDRESS_SCPH30003, BYTE_LANG_DUT },
	{ BIOS_SCPH30003, LANG_PORTUGUESE,	ADDRESS_SCPH30003, BYTE_LANG_POR },

	{ BIOS_SCPH30004R, LANG_ENGLISH,	ADDRESS_SCPH30004R, BYTE_LANG_ENG },
	{ BIOS_SCPH30004R, LANG_FRENCH,		ADDRESS_SCPH30004R, BYTE_LANG_FRA },
	{ BIOS_SCPH30004R, LANG_SPANISH,	ADDRESS_SCPH30004R, BYTE_LANG_SPA },
	{ BIOS_SCPH30004R, LANG_GERMAN,		ADDRESS_SCPH30004R, BYTE_LANG_GER },
	{ BIOS_SCPH30004R, LANG_ITALIAN,	ADDRESS_SCPH30004R, BYTE_LANG_ITA },
	{ BIOS_SCPH30004R, LANG_DUTCH,		ADDRESS_SCPH30004R, BYTE_LANG_DUT },
	{ BIOS_SCPH30004R, LANG_PORTUGUESE,	ADDRESS_SCPH30004R, BYTE_LANG_POR },

	{ BIOS_SCPH39004, LANG_ENGLISH,		ADDRESS_SCPH39004, BYTE_LANG_ENG },
	{ BIOS_SCPH39004, LANG_FRENCH,		ADDRESS_SCPH39004, BYTE_LANG_FRA },
	{ BIOS_SCPH39004, LANG_SPANISH,		ADDRESS_SCPH39004, BYTE_LANG_SPA },
	{ BIOS_SCPH39004, LANG_GERMAN,		ADDRESS_SCPH39004, BYTE_LANG_GER },
	{ BIOS_SCPH39004, LANG_ITALIAN,		ADDRESS_SCPH39004, BYTE_LANG_ITA },
	{ BIOS_SCPH39004, LANG_DUTCH,		ADDRESS_SCPH39004, BYTE_LANG_DUT },
	{ BIOS_SCPH39004, LANG_PORTUGUESE,	ADDRESS_SCPH39004, BYTE_LANG_POR },

	{ BIOS_SCPH50003, LANG_ENGLISH,		ADDRESS_SCPH50003, BYTE_LANG_ENG },
	{ BIOS_SCPH50003, LANG_FRENCH,		ADDRESS_SCPH50003, BYTE_LANG_FRA },
	{ BIOS_SCPH50003, LANG_SPANISH,		ADDRESS_SCPH50003, BYTE_LANG_SPA },
	{ BIOS_SCPH50003, LANG_GERMAN,		ADDRESS_SCPH50003, BYTE_LANG_GER },
	{ BIOS_SCPH50003, LANG_ITALIAN,		ADDRESS_SCPH50003, BYTE_LANG_ITA },
	{ BIOS_SCPH50003, LANG_DUTCH,		ADDRESS_SCPH50003, BYTE_LANG_DUT },
	{ BIOS_SCPH50003, LANG_PORTUGUESE,	ADDRESS_SCPH50003, BYTE_LANG_POR },

	{ BIOS_SCPH50004, LANG_ENGLISH,		ADDRESS_SCPH50004, BYTE_LANG_ENG },
	{ BIOS_SCPH50004, LANG_FRENCH,		ADDRESS_SCPH50004, BYTE_LANG_FRA },
	{ BIOS_SCPH50004, LANG_SPANISH,		ADDRESS_SCPH50004, BYTE_LANG_SPA },
	{ BIOS_SCPH50004, LANG_GERMAN,		ADDRESS_SCPH50004, BYTE_LANG_GER },
	{ BIOS_SCPH50004, LANG_ITALIAN,		ADDRESS_SCPH50004, BYTE_LANG_ITA },
	{ BIOS_SCPH50004, LANG_DUTCH,		ADDRESS_SCPH50004, BYTE_LANG_DUT },
	{ BIOS_SCPH50004, LANG_PORTUGUESE,	ADDRESS_SCPH50004, BYTE_LANG_POR },

	{ BIOS_SCPH70004, LANG_ENGLISH,		ADDRESS_SCPH70004, BYTE_LANG_ENG },
	{ BIOS_SCPH70004, LANG_FRENCH,		ADDRESS_SCPH70004, BYTE_LANG_FRA },
	{ BIOS_SCPH70004, LANG_SPANISH,		ADDRESS_SCPH70004, BYTE_LANG_SPA },
	{ BIOS_SCPH70004, LANG_GERMAN,		ADDRESS_SCPH70004, BYTE_LANG_GER },
	{ BIOS_SCPH70004, LANG_ITALIAN,		ADDRESS_SCPH70004, BYTE_LANG_ITA },
	{ BIOS_SCPH70004, LANG_DUTCH,		ADDRESS_SCPH70004, BYTE_LANG_DUT },
	{ BIOS_SCPH70004, LANG_PORTUGUESE,	ADDRESS_SCPH70004, BYTE_LANG_POR },

	{ BIOS_SCPH75004, LANG_ENGLISH,		ADDRESS_SCPH75004, BYTE_LANG_ENG },
	{ BIOS_SCPH75004, LANG_FRENCH,		ADDRESS_SCPH75004, BYTE_LANG_FRA },
	{ BIOS_SCPH75004, LANG_SPANISH,		ADDRESS_SCPH75004, BYTE_LANG_SPA },
	{ BIOS_SCPH75004, LANG_GERMAN,		ADDRESS_SCPH75004, BYTE_LANG_GER },
	{ BIOS_SCPH75004, LANG_ITALIAN,		ADDRESS_SCPH75004, BYTE_LANG_ITA },
	{ BIOS_SCPH75004, LANG_DUTCH,		ADDRESS_SCPH75004, BYTE_LANG_DUT },
	{ BIOS_SCPH75004, LANG_PORTUGUESE,	ADDRESS_SCPH75004, BYTE_LANG_POR },

	{ BIOS_SCPH77004, LANG_ENGLISH,		ADDRESS_SCPH77004, BYTE_LANG_ENG },
	{ BIOS_SCPH77004, LANG_FRENCH,		ADDRESS_SCPH77004, BYTE_LANG_FRA },
	{ BIOS_SCPH77004, LANG_SPANISH,		ADDRESS_SCPH77004, BYTE_LANG_SPA },
	{ BIOS_SCPH77004, LANG_GERMAN,		ADDRESS_SCPH77004, BYTE_LANG_GER },
	{ BIOS_SCPH77004, LANG_ITALIAN,		ADDRESS_SCPH77004, BYTE_LANG_ITA },
	{ BIOS_SCPH77004, LANG_DUTCH,		ADDRESS_SCPH77004, BYTE_LANG_DUT },
	{ BIOS_SCPH77004, LANG_PORTUGUESE,	ADDRESS_SCPH77004, BYTE_LANG_POR },

	{ BIOS_EU_230, LANG_ENGLISH,		ADDRESS_EU_230, BYTE_LANG_ENG },
	{ BIOS_EU_230, LANG_FRENCH,			ADDRESS_EU_230, BYTE_LANG_FRA },
	{ BIOS_EU_230, LANG_SPANISH,		ADDRESS_EU_230, BYTE_LANG_SPA },
	{ BIOS_EU_230, LANG_GERMAN,			ADDRESS_EU_230, BYTE_LANG_GER },
	{ BIOS_EU_230, LANG_ITALIAN,		ADDRESS_EU_230, BYTE_LANG_ITA },
	{ BIOS_EU_230, LANG_DUTCH,			ADDRESS_EU_230, BYTE_LANG_DUT },
	{ BIOS_EU_230, LANG_PORTUGUESE,		ADDRESS_EU_230, BYTE_LANG_POR },


	// US
	{ BIOS_SCPH39001, LANG_ENGLISH,		ADDRESS_SCPH39001, BYTE_LANG_ENG },
	{ BIOS_SCPH39001, LANG_FRENCH,		ADDRESS_SCPH39001, BYTE_LANG_FRA },
	{ BIOS_SCPH39001, LANG_SPANISH,		ADDRESS_SCPH39001, BYTE_LANG_SPA },
	{ BIOS_SCPH39001, LANG_GERMAN,		ADDRESS_SCPH39001, BYTE_LANG_GER },
	{ BIOS_SCPH39001, LANG_ITALIAN,		ADDRESS_SCPH39001, BYTE_LANG_ITA },
	{ BIOS_SCPH39001, LANG_DUTCH,		ADDRESS_SCPH39001, BYTE_LANG_DUT },
	{ BIOS_SCPH39001, LANG_PORTUGUESE,	ADDRESS_SCPH39001, BYTE_LANG_POR },

	{ BIOS_SCPH39001V2, LANG_ENGLISH,	ADDRESS_SCPH39001V2, BYTE_LANG_ENG },
	{ BIOS_SCPH39001V2, LANG_FRENCH,	ADDRESS_SCPH39001V2, BYTE_LANG_FRA },
	{ BIOS_SCPH39001V2, LANG_SPANISH,	ADDRESS_SCPH39001V2, BYTE_LANG_SPA },
	{ BIOS_SCPH39001V2, LANG_GERMAN,	ADDRESS_SCPH39001V2, BYTE_LANG_GER },
	{ BIOS_SCPH39001V2, LANG_ITALIAN,	ADDRESS_SCPH39001V2, BYTE_LANG_ITA },
	{ BIOS_SCPH39001V2, LANG_DUTCH,		ADDRESS_SCPH39001V2, BYTE_LANG_DUT },
	{ BIOS_SCPH39001V2, LANG_PORTUGUESE,ADDRESS_SCPH39001V2, BYTE_LANG_POR },

	{ BIOS_SCPH70012, LANG_ENGLISH,		ADDRESS_SCPH70012, BYTE_LANG_ENG },
	{ BIOS_SCPH70012, LANG_FRENCH,		ADDRESS_SCPH70012, BYTE_LANG_FRA },
	{ BIOS_SCPH70012, LANG_SPANISH,		ADDRESS_SCPH70012, BYTE_LANG_SPA },
	{ BIOS_SCPH70012, LANG_GERMAN,		ADDRESS_SCPH70012, BYTE_LANG_GER },
	{ BIOS_SCPH70012, LANG_ITALIAN,		ADDRESS_SCPH70012, BYTE_LANG_ITA },
	{ BIOS_SCPH70012, LANG_DUTCH,		ADDRESS_SCPH70012, BYTE_LANG_DUT },
	{ BIOS_SCPH70012, LANG_PORTUGUESE,	ADDRESS_SCPH70012, BYTE_LANG_POR },

	{ BIOS_SCPH77001, LANG_ENGLISH,		ADDRESS_SCPH77001, BYTE_LANG_ENG },
	{ BIOS_SCPH77001, LANG_FRENCH,		ADDRESS_SCPH77001, BYTE_LANG_FRA },
	{ BIOS_SCPH77001, LANG_SPANISH,		ADDRESS_SCPH77001, BYTE_LANG_SPA },
	{ BIOS_SCPH77001, LANG_GERMAN,		ADDRESS_SCPH77001, BYTE_LANG_GER },
	{ BIOS_SCPH77001, LANG_ITALIAN,		ADDRESS_SCPH77001, BYTE_LANG_ITA },
	{ BIOS_SCPH77001, LANG_DUTCH,		ADDRESS_SCPH77001, BYTE_LANG_DUT },
	{ BIOS_SCPH77001, LANG_PORTUGUESE,	ADDRESS_SCPH77001, BYTE_LANG_POR },

	{ BIOS_SCPH90001, LANG_ENGLISH,		ADDRESS_SCPH90001, BYTE_LANG_ENG },
	{ BIOS_SCPH90001, LANG_FRENCH,		ADDRESS_SCPH90001, BYTE_LANG_FRA },
	{ BIOS_SCPH90001, LANG_SPANISH,		ADDRESS_SCPH90001, BYTE_LANG_SPA },
	{ BIOS_SCPH90001, LANG_GERMAN,		ADDRESS_SCPH90001, BYTE_LANG_GER },
	{ BIOS_SCPH90001, LANG_ITALIAN,		ADDRESS_SCPH90001, BYTE_LANG_ITA },
	{ BIOS_SCPH90001, LANG_DUTCH,		ADDRESS_SCPH90001, BYTE_LANG_DUT },
	{ BIOS_SCPH90001, LANG_PORTUGUESE,	ADDRESS_SCPH90001, BYTE_LANG_POR },

	{ BIOS_USA_230, LANG_ENGLISH,		ADDRESS_USA_230, BYTE_LANG_ENG },
	{ BIOS_USA_230, LANG_FRENCH,		ADDRESS_USA_230, BYTE_LANG_FRA },
	{ BIOS_USA_230, LANG_SPANISH,		ADDRESS_USA_230, BYTE_LANG_SPA },
	{ BIOS_USA_230, LANG_GERMAN,		ADDRESS_USA_230, BYTE_LANG_GER },
	{ BIOS_USA_230, LANG_ITALIAN,		ADDRESS_USA_230, BYTE_LANG_ITA },
	{ BIOS_USA_230, LANG_DUTCH,			ADDRESS_USA_230, BYTE_LANG_DUT },
	{ BIOS_USA_230, LANG_PORTUGUESE,	ADDRESS_USA_230, BYTE_LANG_POR },

	//JAP
	{ BIOS_JAP_230, LANG_ENGLISH,		ADDRESS_JAP_230, BYTE_LANG_ENG },
	{ BIOS_JAP_230, LANG_JAPANESE,		ADDRESS_JAP_230, BYTE_LANG_JAP },

	//ASIA
	{ BIOS_ASIA_230, LANG_ENGLISH,		ADDRESS_ASIA_230, BYTE_LANG_ENG },
	{ BIOS_ASIA_230, LANG_CHINESE,		ADDRESS_ASIA_230, BYTE_LANG_TCH },

	
};



namespace LanguageInjector
{
	
	void Inject(std::string bios_path, const char* language) {

		u32 version, region;
		std::string description, zone;
		std::string bios_file = bios_path;
		if (!IsBIOS(bios_file.c_str(), version, description, region, zone)) {
			Console.WriteLn("Invalid BIOS file: %s \n", bios_path.c_str());
			return;
		}
		std::string bios_descr = description;

		bios_descr.erase(remove(bios_descr.begin(), bios_descr.end(), ' '), bios_descr.end());
		Console.WriteLn("Detected BIOS: %s \n", bios_descr.c_str());

		size_t last_slash_index = bios_path.find_last_of("\\/");
		std::string bios_name = bios_path.substr(last_slash_index + 1, bios_path.length());
		Console.WriteLn("BIOS file name: %s\n", bios_name.c_str());

		bios_lang lang_data = _GetLanguageDataForBios(bios_descr.c_str(), language);

		if (lang_data.bios_name == NULL) {
			Console.WriteLn("No injection data found for Bios %s - language: %s \n", bios_name.c_str(), language);
		}
		else
		{
			if (_ModifyNVM(bios_path, lang_data))
				Console.WriteLn("Injected successfully language data for Bios %s - language: %s \n", bios_name.c_str(), language);
		}

	}


	bios_lang _GetLanguageDataForBios(const char* bios_name, const char* language) {

		for (int i = 0; i < NUM_BIOS_LANG_ENTRIES; i++) {
			if (!strcmp(bios_language[i].bios_name, bios_name) && !strcmp(bios_language[i].language, language)) {
				return bios_language[i];
			}
		}
		return { NULL, NULL, NULL, NULL };

	}


	bool _ModifyNVM(std::string bios_path, bios_lang lang_data) {

		size_t lastindex = bios_path.find_last_of(".");

		std::string rawpath = bios_path.substr(0, lastindex);
		std::string rawext = bios_path.substr(lastindex + 1, bios_path.length());
		std::string nvm_ext = ".nvm";

		if (_FirstUpper(rawext))
			nvm_ext = ".NVM";
		
		std::string path_nvm = rawpath + nvm_ext;
		if (FileSystem::FileExists(path_nvm.c_str()))
		{
			//std::string path_nvm_output = rawpath + "_DEBUG_OUT" + nvm_ext;
			std::string path_nvm_output = rawpath + nvm_ext;

			const int BUFFER_SIZE = 1024;
			uint8_t buffer[BUFFER_SIZE];
			std::ifstream infile(path_nvm, std::ifstream::binary);
			infile.read((char*)buffer, BUFFER_SIZE);
			infile.close();
			//Console.WriteLn("File read in buffer\n", buffer);

			if (_ModifyLanguageOptionByte(buffer, lang_data)) {
				std::ofstream outfile(path_nvm_output, std::ofstream::binary);
				outfile.write((char*)buffer, BUFFER_SIZE);                      // write the actual text
				outfile.close();
				return true;
			}
		}
		return false;

	}


	bool _ModifyLanguageOptionByte(uint8_t buffer[], bios_lang lang_data) {

		size_t opt_index = (size_t)lang_data.address * 16;
		Console.WriteLn("Checking language byte, found: %x\n", buffer[opt_index + 1]);

		if (buffer[opt_index + 1] == lang_data.byte_lang) {
			Console.WriteLn("Language already set to %s, no need to inject language byte\n", lang_data.language);
		}
		else
		{
			buffer[opt_index + 1] = lang_data.byte_lang;
			Console.WriteLn("Language set to %s\n", lang_data.language);
			int checksum = 0;
			for (size_t i = opt_index; i < opt_index + 15; i++) {
				checksum += buffer[i] % 256;
				//Console.WriteLn("Calc checksum adding value: %x\n", buffer[i]);
			}
			buffer[opt_index + 15] = checksum;
			Console.WriteLn("checksum byte set\n");
			return true;
		}
		return false;
	}


	bool _FirstUpper(const std::string& word) {
		return word.size() && std::isupper(word[0]);
	}



} // closing namespace
