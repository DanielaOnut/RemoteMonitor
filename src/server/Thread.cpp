#include "Thread.h"

Thread::Thread(Server * server, QByteArray & request, QSslSocket * socket) {
    this->server = server;
    this->socket = socket;    
    this->requestData = request;
}

pair Thread::getJwt(const QByteArray & data) {
    int deviceIndex = data.indexOf("device ->") + sizeof("device ->");
    int enterIndex = data.indexOf("\nmachine");
    QByteArray deviceName = data.mid(deviceIndex, enterIndex - deviceIndex);
    
    int machineIndex = data.indexOf("machine ->") + sizeof("machine ->");
    QByteArray machineName = data.data() + machineIndex;
    
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces); 
    QMetaObject::invokeMethod(this->server, [this, id, deviceName] {
        this->server->updateClientsMap(id, deviceName);
    });

    auto token = jwt::create()
        .set_type("JWS")
        .set_issuer("remoteserver")
        .set_payload_claim("id", jwt::claim(std::string(id.toStdString())))
        .set_payload_claim("device", jwt::claim(std::string(deviceName)))
        .set_payload_claim("server name", jwt::claim(std::string(machineName)))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(1))
        // .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(4))
        .sign(jwt::algorithm::hs256{"remote-monitor-project-super-key"});

    return {token.c_str(), "200 OK"};
}

pair Thread::updateJwt (const QByteArray & request) {
    int tokenIndex = request.indexOf("Authorization: Bearer") + sizeof ("Authorization: Bearer");
    QByteArray token = request.data() + tokenIndex;
    token = token.split('\n')[0].trimmed();

    auto decoded = jwt::decode(token.toStdString());
    try {
        auto verifier = jwt::verify()
            .with_issuer("remoteserver")
            .allow_algorithm(jwt::algorithm::hs256{"remote-monitor-project-super-key"});

        verifier.verify(decoded);
    }
    catch (jwt::error::token_verification_exception & e) {
        QString msg = "Failed: ";
        msg.append(e.what());

        if (!msg.contains("token expired")) 
            return {msg.toStdString(), "401 Unauthorized"};
    }
    std::string id = decoded.get_payload_claim("id").as_string();
    if (!this->server->clientExists(id))
        return {"Failed: no client exists with this id", "401 Unauthorized"};
   
    auto deviceName = decoded.get_payload_claim("device").as_string();
    if (this->server->getDeviceNameByClient(id.c_str()) != deviceName.c_str())
        return {"Failed: device name doesn't match", "401 Unauthorized"};

    auto machineName = decoded.get_payload_claim("server name").as_string();
    auto newToken = jwt::create()
        .set_type("JWS")
        .set_issuer("remoteserver")
        .set_payload_claim("id", jwt::claim(std::string(id)))
        .set_payload_claim("device", jwt::claim(std::string(deviceName)))
        .set_payload_claim("server name", jwt::claim(std::string(machineName)))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(1))
        // .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(4))
        .sign(jwt::algorithm::hs256{"remote-monitor-project-super-key"});

    return {newToken.c_str(), "200 OK"};
}

pair Thread::verifyToken(const QByteArray &request) {
    if (!request.contains("Authorization: Bearer"))
        return {"Failed: no token is present in header", "401 Unauthorized"};

    int tokenIndex = request.indexOf("Authorization: Bearer") +
                     QByteArray("Authorization: Bearer").length();
    QByteArray token = request.mid(tokenIndex).split('\n')[0].trimmed();

    auto decoded = jwt::decode(token.toStdString());
    if (!decoded.has_payload_claim("id"))
        return {"Failed: jwt token doesn't have any claim named 'id'", "401 Unauthorized"};

    std::string id = decoded.get_payload_claim("id").as_string();
    if (!this->server->clientExists(id))
        return {"Failed: no client exists with this id", "401 Unauthorized"};

    try {
        auto verifier = jwt::verify()
            .with_issuer("remoteserver")
            .with_claim("id", jwt::claim(id))
            .allow_algorithm(jwt::algorithm::hs256{"remote-monitor-project-super-key"});

        verifier.verify(decoded);
    }
    catch (const jwt::error::token_verification_exception &e) {
        return {std::string("Failed: ") + e.what(), "401 Unauthorized"};
    }
    catch (const jwt::error::invalid_json_exception &e) {
        return {std::string("Failed: ") + e.what(), "401 Unauthorized"};
    }
    catch (const jwt::error::signature_verification_error &) {
        return {"Failed: Token signature verification failed", "401 Unauthorized"};
    }

    try {
        auto exp = decoded.get_payload_claim("exp").as_date();
        if (std::chrono::system_clock::now() > exp)
            return {"Failed: token expired", "401 Unauthorized"};
    }
    catch (const std::exception &e) {
        return {std::string("Failed: invalid or missing exp claim (") + e.what() + ")", "401 Unauthorized"};
    }

    return {"valid token", "200 OK"};
}


pair Thread::updateStats (const QByteArray & request) {
    pair response = this->verifyToken(request);
    if (response.first.contains("Failed"))
        return response;

    response = {this->getCpuUsage(), "200 OK"};
    response.first.append(this->getRamUsage());
    response.first.append(this->getDiskUsage());
    response.first.append(this->getProcessesList());

    return response;
}

std::string Thread::getCpuUsage() {
    std::ifstream fin ("/proc/stat");
    char buffer[500], *p, *pEnd;
    fin.getline(buffer, 500);
    char* pos = strstr(buffer, "cpu  ");
    memmove(pos, pos + strlen("cpu  "), strlen(pos + strlen("cpu  ")) + 1);

    p = strtok (buffer, " ");
    int counter = 0, totalCpu = 0, workCpu = 0;
    while (p) {
        int value = strtoll (p, &pEnd, 10);
        if (counter < 3) {
            workCpu += value;
            counter++;
        }
        totalCpu += value;
        p = strtok (NULL, " ");
    }
    
    std::string response = "CPU: ";
    response.append(std::to_string(totalCpu));
    response.append(" ").append(std::to_string(workCpu))
            .append("\n");
    return response;
}

std::string Thread::getRamUsage() {
    std::ifstream fin ("/proc/meminfo");
    std::string key, unit, value;
    std::string response = "RAM: ";
    
    while (fin >> key >> value >> unit) {
        if (key == "MemTotal:") {
            response.append(value).append(" ");
        } else if (key == "MemAvailable:") {
            response.append(value).append("\n");
            break; 
        }
    }
    
    return response;
}

std::string Thread::getDiskUsage() {
    struct statvfs diskData;
    if (statvfs("/", &diskData) != 0) {
        std::cerr << "Failed to get disk info\n";
        return "";
    }

    double total = (double)diskData.f_blocks * diskData.f_frsize / (1024.0 * 1024.0 * 1024.0);
    double free  = (double)diskData.f_bfree  * diskData.f_frsize / (1024.0 * 1024.0 * 1024.0);
    double used  = total - free;

    std::string response = "Disk: ";
    response.append(QString::number(total, 'f', 1).toStdString()).append(" ")
            .append(QString::number(used, 'f', 1).toStdString())
            .append("\n");

    return response;
}

std::string Thread::getProcessesList() {
    std::string response;

    for (const auto &dirEntry : dir_iterator("/proc")) 
        if (dirEntry.is_directory()) {
            std::string path = dirEntry.path();
            std::string pid = path.substr(path.find("proc/") + 5);
            int pidVal = 0;
            if (this->isStringANumber(pid))
                pidVal = std::stol(pid);

            if (pidVal > 1100) {
                std::ifstream fin (path + "/comm");
                std::string procName;
                std::getline(fin, procName);
                if (procName.empty()) continue;
                procName.erase(std::remove_if(procName.begin(), procName.end(), isspace), procName.end());

                std::ifstream cpuInfoPath (path + "/stat");
                std::string statBuffer, field, utime, stime;
                std::getline(cpuInfoPath, statBuffer);
                if (statBuffer.empty()) continue;

                std::istringstream iss(statBuffer.substr(statBuffer.find_last_of(')') + 2));
                int fieldCounter = 3;
                while (iss >> field) {
                    fieldCounter++;
                    if (fieldCounter == 14)
                        utime = field;
                    if (fieldCounter == 15) {
                        stime = field;
                        break;
                    }
                }
                if (utime.empty() || stime.empty()) continue;

                response.append("Proc: " + pid + " ");
                response.append(procName).append(" ");
                response.append(utime).append(" ") 
                        .append(stime).append(" "); 

                std::ifstream ramFile (path + "/smaps");
                std::string key, value, unit;
                int pssSum = 0;
                while (ramFile >> key >> value >> unit) 
                    if (key.contains("Pss:")) {
                        pssSum += std::stol(value);
                    }
                response.append(std::to_string(pssSum))
                        .append("\n");
            }
        }
    // std::cout << response;
    return response;
}

bool Thread::isStringANumber (const std::string &s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}

pair Thread::getSysInfo(const QByteArray & request) {
    pair response = this->verifyToken(request);
    if (response.first.contains("Failed"))
        return response;

    std::ifstream fin ("/etc/os-release");
    char osName[500], osVersion[500];
    fin.getline (osName, 500);
    fin.getline (osVersion, 500);
    struct utsname cpuArch;
    uname (&cpuArch);

    response.first = osName;
    response.first.append("\n");
    response.first.append(osVersion);
    response.first.append("\n");
    response.first.append(cpuArch.machine);
    response.first.append("\n");
    return response;
}

void Thread::run () {
    std::cout << "\nReceived request:\n" << this->requestData.data() << "\n";
    
    pair msg;
    if (requestData.contains("/register")) 
        msg = this->getJwt(requestData);
    else if (requestData.contains("/updatejwt"))
        msg = this->updateJwt(requestData);
    else if (requestData.contains("/updateall"))
        msg = this->updateStats(requestData);
    else if (requestData.contains("/sysinfo"))
        msg = this->getSysInfo(requestData);

    QMetaObject::invokeMethod(this->server, [this, msg] {
        QByteArray response = this->server->getHttpResponse(msg.first.c_str(), msg.second);
        this->server->sendResponse(this->socket, response);
    });
}

Thread::~Thread() {
    
}