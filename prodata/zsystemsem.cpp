#include "zsystemsem.h"
#include <openssl/sha.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include "zfile.h"





ZSystemSem::ZSystemSem():m_createdFile(false)
{
}


inline string ZSystemSem::makeKeyFileName() const
{
    if(m_key.empty())
        return string();
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, m_key.c_str(), m_key.size());
    SHA256_Final(hash, &sha256);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    string retString = "/tmp/zipc_systemsem_" + ss.str();
    return retString;
}

int  ZSystemSem::createUnixKeyFile(const string &fileName)
{
    if(ZFile::exists(fileName))
        return 0;
    int fd = open(fileName.c_str(), O_EXCL | O_CREAT | O_RDWR, 0640);

    if(-1 ==fd)
    {
        if(errno == EEXIST)
            return 0;
        return -1;
    }
    else
    {
        close(fd);
    }
    return 1;

}

key_t ZSystemSem::semFileHandle(AccessMode mode)
{
    if(-1 != m_semKey)
        return m_semKey;
    if(m_key.empty())
    {
        zprintf1("ZSystemSem m_key empty!\n");
        return -1;
    }
    int built = createUnixKeyFile(m_fileName);
    if(-1 == built)
    {
        zprintf1("unable to make key %s!\n", m_key.c_str());
        return -1;
    }

    m_createdFile = (1 == built);

    m_semKey = ftok(m_fileName.c_str(), 'Z');
    if(-1 == m_semKey)
    {
        zprintf1("ftok failed %s!\n", m_fileName.c_str());
        return -1;
    }
    int ret = handle(mode);
    if(ret < 0)
    {
        zprintf1("ZSystemSem create key %d error!\n", m_semKey);
        semFileCleanHandle();
        return -1;
    }
    if(mode == Create)
        m_createdFile = true;

    return m_semKey;
}

void ZSystemSem::semFileCleanHandle()
{
    m_semKey = -1;
    if(m_createdFile)
    {
        ZFile::remove(m_fileName);
        m_createdFile = false;
    }
    cleanHandle();
}

int ZSystemSem::setKey(const string &key, int initVal, AccessMode mode)
{
    if(key == m_key && mode == Open)
    {
        zprintf1("%s is have!\n", key.c_str());
        return -1;
    }
    if(key == m_key && mode == Create && m_created && m_createdFile)
    {
        m_initVal = initVal;
        m_semKey = -1;
        return semFileHandle(mode);
    }

    semFileCleanHandle();

    m_key = key;
    m_initVal = initVal;
    m_fileName = makeKeyFileName();
    return semFileHandle(mode);
}

string ZSystemSem::key() const
{
    return m_key;
}


