#ifndef spiffs_api_h
#define spiffs_api_h

/*
 spiffs_api.h - file system wrapper for SPIFFS
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
#include <limits>
#include "FS.h"
#undef max
#undef min
#include "FSImpl.h"
#include "spiffs/spiffs.h"
//#include "debug.h"

#include <io.h>

#define DEBUGV  printf

using namespace fs;

extern int32_t spiffs_hal_write(uint32_t addr, uint32_t size, uint8_t *src);
extern int32_t spiffs_hal_erase(uint32_t addr, uint32_t size);
extern int32_t spiffs_hal_read(uint32_t addr, uint32_t size, uint8_t *dst);

int getSpiffsMode(OpenMode openMode, AccessMode accessMode);
bool isSpiffsFilenameValid(const char* name);

class SPIFFSFileImpl;
class SPIFFSDirImpl;

class SPIFFSImpl : public FSImpl
{
public:
    SPIFFSImpl()
    {
    }

    FileImplPtr open(const char* path, OpenMode openMode, AccessMode accessMode) override;
    bool exists(const char* path) override;
    DirImplPtr openDir(const char* path) override;

    bool rename(const char* pathFrom, const char* pathTo) override
    {
        return false;
    }
    bool info(FSInfo& info) override
    {
        return false;
    }

    bool remove(const char* path) override
    {
        return false;
    }

    bool begin() override
    {
        return true;
    }

    bool format() override
    {
        // TODO: Delete all files and directories if spiffs directory exists

        // TODO: Create spiffs directory
        return true;
    }

protected:
    friend class SPIFFSFileImpl;
    friend class SPIFFSDirImpl;

    uint32_t _start;
    uint32_t _size;
    uint32_t _pageSize;
    uint32_t _blockSize;
    uint32_t _maxOpenFds;

    std::unique_ptr<uint8_t[]> _workBuf;
    std::unique_ptr<uint8_t[]> _fdsBuf;
    std::unique_ptr<uint8_t[]> _cacheBuf;
};

#define CHECKFD() while (_fd == 0) { panic(); }

class SPIFFSFileImpl : public FileImpl
{
public:
    SPIFFSFileImpl(SPIFFSImpl* fs, int fd)
        : _fs(fs)
        , _fd(fd)
        , _fp(NULL)
    , _written(false)
    {
        _getStat();
    }

    ~SPIFFSFileImpl() override
    {
        close();
    }

    size_t write(const uint8_t *buf, size_t size) override
    {
        return _write(_fd, buf, size);
    }

    size_t read(uint8_t* buf, size_t size) override
    {
        return _read(_fd, buf, size);
    }

    void flush() override
    {
        if (!_fp)
        {
            _fp = _fdopen(_fd, "w");
        }
        if (_fp)
        {
            fflush(_fp);
        }
    }

    bool seek(uint32_t pos, SeekMode mode) override
    {
        long ret = _lseek(_fd, pos, mode);
        return true;
    }

    size_t position() const override
    {
        return _tell(_fd);
    }

    size_t size() const override
    {
        return _filelength(_fd);
    }

    void close() override
    {
        DEBUGV("SPIFFS_close: fd=%d\r\n", _fd);
        if (_fp)
        {
            fclose(_fp);
        }
        else if (_fd != -1)
        {
            _close(_fd);
        }
        _fp = NULL;
        _fd = -1;
    }

    const char* name() const override
    {
        return (const char*) "";// TODO _stat.name;
    }

protected:
    void _getStat() const
    {
        _written = false;
    }

    SPIFFSImpl* _fs;
    int _fd;
    mutable bool        _written;
    FILE* _fp;
};

class SPIFFSDirImpl : public DirImpl
{
public:
    SPIFFSDirImpl(const String& pattern, SPIFFSImpl* fs)
        : _pattern(pattern)
        , _fs(fs)
        , _valid(false)
    {
    }

    ~SPIFFSDirImpl() override
    {
    }

    FileImplPtr openFile(OpenMode openMode, AccessMode accessMode) override
    {
        return std::make_shared<SPIFFSFileImpl>(_fs, -1);
    }

    const char* fileName() override
    {
        return (const char*)"";// TODO _dirent.name;
    }

    size_t fileSize() override
    {
        if (!_valid) {
            return 0;
        }

        return 0;// TODO _dirent.size;
    }

    bool next() override
    {
        return false;
    }

protected:
    String _pattern;
    SPIFFSImpl* _fs;
    bool _valid;
};


#endif//spiffs_api_h
