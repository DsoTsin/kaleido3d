#pragma once

#include <Interface/IIODevice.h>
#include <string>
#include <mutex>
#include <cstdio>
#include <Config/OSHeaders.h>


#if defined(K3DPLATFORM_OS_IOS) | defined(K3DPLATFORM_OS_MAC)
//#import <Foundation/Foundation.h>
#endif


namespace k3d {
	
	const char *header = 
		"<!DOCTYPE html>\n<html>\n"
		"<head>\n\t<meta charset=\"utf-8\"/><title>kaleido3d log</title>"
		"\n\t<link rel=\"stylesheet\" href=\"style.css\">\n</head>"
		"\n<body>\n\t<nav id=\"nav\" class=\"nav\">\n\t\t"
		"\t\t<div class=\"alignright\"><ul class =\"menu\">\n\t\t\t<li><a>Home</a></li></ul>\n\t\t</div>"
		"\n\t</nav>\n\t<section id=\"homepage\" class=\"fullscreen\">\n\t\t<div class=\"container claim\">\n\t\t\t<div class=\"span12\">"
		"\n\t\t\t\t<h1 class=\"aligncenter\">Kaleido3D</h1>\n\t\t\t</div>\n\t\t</div>\n\t\t<div class=\"container tmpMargin textOstec\">\n\t\t\t<div class =\"span12\">"
		;

	const char * tail = "\n\t\t\t</div>\n\t\t</div>\n\t</section>\n\t<footer>\n\t<style>{margin:0px; padding:0px; }</style>\n\t</footer>\n</body>\n</html>";

	static inline void OutputStr2IODevice(IIODevice* device, const char *str)
	{
		if (device != nullptr) {
			while (char c = *str++)
				device->Write(&c, 1);
			//Need Flush Data EveryTime?
			//device->Flush();
		}
	}

	static inline const char* GetLocalTime() {
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		static char time_info[128] = { 0 };
		::sprintf(time_info, "%02d:%02d:%02d ", tm->tm_hour, tm->tm_min, tm->tm_sec);
		return time_info;
	}

	static inline std::string Txt2Html(const char *str) {
		std::string ret(GetLocalTime());
		const char *s = str;
		while (*s) {
			if (*s == '\t') {
				ret += "&nbsp; &nbsp; ";
			}
			else if (*s == '<') {
				ret += "&lt;";
			}
			else if (*s == '>') {
				ret += "&gt;";
			}
			else if (*s == '\n') {
				if (*(s + 1)) ret += "<br/>" + std::string(GetLocalTime());
			}
			else {
				ret += *s;
			}
			s++;
		}
		return ret;
	}

}