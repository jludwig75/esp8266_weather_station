/*
 spiffs_api.cpp - file system wrapper for SPIFFS
 Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.

 This code was influenced by NodeMCU and Sming libraries, and first version of
 Arduino wrapper written by Hristo Gochkov.

 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "spiffs_api.h"

#include <string>

#include <io.h>
#include <fcntl.h>

using namespace fs;
using namespace std;

const string spiffs_root = "spiffs_fs";


static int convert_open_flags(OpenMode openMode, AccessMode accessMode)
{
    int open_flags = 0;

    if (openMode & OM_CREATE)
    {
        open_flags |= _O_CREAT;
    }
    if (openMode & OM_APPEND)
    {
        open_flags |= _O_APPEND;
    }
    if (openMode & OM_TRUNCATE)
    {
        open_flags |= _O_TRUNC;
    }

    switch (accessMode)
    {
    case AM_READ:
        open_flags |= _O_RDONLY;
        break;
    case AM_WRITE:
        open_flags |= _O_WRONLY;
        break;
    case AM_RW:
        open_flags |= _O_RDWR;
        break;
    }

    return open_flags;
}


FileImplPtr SPIFFSImpl::open(const char* path, OpenMode openMode, AccessMode accessMode)
{
    string file_name = spiffs_root + "/" + path;

    int fd = _open(file_name.c_str(), convert_open_flags(openMode, accessMode), _S_IREAD | _S_IWRITE);
    if (fd == -1)
    {
        return NULL;
    }

    return std::make_shared<SPIFFSFileImpl>(this, fd);
}

bool SPIFFSImpl::exists(const char* path)
{
    return false;
}

DirImplPtr SPIFFSImpl::openDir(const char* path)
{
    return std::make_shared<SPIFFSDirImpl>(path, this);
}

FS SPIFFS = FS(FSImplPtr(new SPIFFSImpl));
