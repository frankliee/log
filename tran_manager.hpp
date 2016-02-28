/*
 * Copyright [2012-2015] DaSE@ECNU
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * /log/trans_manager.hpp
 *
 *  Created on: 2016年2月25日
 *      Author: imdb
 *		   Email: 
 * 
 * Description:
 *
 */

#ifndef TRAN_MANAGER_HPP_
#define TRAN_MANAGER_HPP_

#include <vector>
#include <tuple>
#include <iostream>
#include <atomic>
#include <memory>
#include <map>
#include <utility>
#include <unordered_map>
#include <time.h>
#include <sys/time.h>
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "log_manager.hpp"



using std::vector;
using std::tuple;
using std::get;
using std::atomic;
using std::string;
using std::shared_ptr;
using std::make_shared;
using std::atomic_is_lock_free;
using std::map;
using std::pair;
using std::unordered_map;
using UInt64 = unsigned long long;
using UInt32 = unsigned int;
using UInt16 = unsigned short;
using RetCode = int;
const int kWrite = 1;
const int kRead = 2;
const char kUVisible = 0;
const char kVisible = 1;

const int kInfinite = 10000;

const int kLocalTran = 1;
const int kRemoteTran = 2;

const UInt16 kDefaultTranPort = 8082;

class Strip {
 public:
  UInt64 PartId;
  UInt64 Pos;
  UInt64 Offset;
  Strip():PartId(0),Pos(0),Offset(0) {}
};


class Tran{
 public:
  UInt64 TransId = 0;
  int Type = kRead;
  char Visible = kUVisible;
  atomic<int> NPart;
  atomic<int> NCommit;
  atomic<int> NAbort;
  vector<Strip *> StripList;
  shared_ptr<Tran> NextPtr;
  Tran() { Init(kInfinite, 0, 0);}

  Tran(UInt64 transId,int type,UInt32 nPart):TransId(transId),Type(type){
    Init(nPart, 0, 0);
  }

  void Init(int nPart, int nCommit, int nAbort) {
    NPart = nPart;
    NCommit = nCommit;
    NAbort = nAbort;
    NextPtr = nullptr;
  }
  void Commit(){
    NCommit++;
    if (NCommit + NAbort == NPart && NAbort==0)
        Visible = kVisible;
  }
  void Abort() {
    NAbort++;
  }
  ~Tran(){
    for (auto strip:StripList)
      delete strip;
  }
};
using TranPtr = shared_ptr<Tran>;

class Checkpoint{
  UInt64 MemoryPos;
  UInt64 HdfsPos;
};

class TranManager{
 public:
  atomic<UInt64> TId;
  atomic<TranPtr> WriteHeadPtr;
  atomic<TranPtr> ReadHeadPtr;
  atomic<int> CommitCount;
  atomic<int> AbortPreCount;
  unordered_map<UInt64,Checkpoint> CheckpointList;
  unordered_map<UInt64,atomic<UInt64>> PosList;
};

class TranService{
 public:
  TranManager TM;
  UInt16 port = kDefaultTranPort;
   static TranService & getInstance(){
     static TranService trans_service;
     return trans_service;
    }
   static void SetPort(UInt16 port) {
     TranService & ts = TranService::getInstance();
     ts.port = port;
   }
   static void Startup() {
     RecoveryFromLog();
     LogManager::Startup();
     pthread_t pid;
     pthread_create(&pid, NULL, MainThread, nullptr);
     sleep(1);
   }
 private:
   static void RecoveryFromLog();
   static void * MainThread(void * arg){
     caf::spawn(WebServiceBehav);
     caf::spawn(GarbageCollectBehav);
     caf::spawn(LogDeleteBehav);
     caf::await_all_actors_done();
   }

 private:
   static void WebServiceBehav(caf::event_based_actor * self);
   static void GarbageCollectBehav(caf::event_based_actor * self);
   static void LogDeleteBehav(caf::event_based_actor * self);
   TranService(){}
   TranService(const TranService & tran_service) {}
   TranService & operator = (const TranService & trans_service){}
};
struct SnapshortPart {
  UInt64 CheckpointPos;
  vector<Strip> StripList;
};
struct Snapshort {
  map<UInt64,UInt64> CheckpointList;
  vector<Strip> StripList;
};

class TranAPIs{
  class Local{
    static TranPtr CreateWriteTran(vector<pair<UInt64,UInt64>> & partList);
    static RetCode CommitWriteTran(TranPtr transPtr);
    static RetCode AbortWriteTran(TranPtr transPtr);
    static TranPtr CreateReadTran(vector<UInt64> & partList);
    static RetCode EndReadTran(TranPtr transPtr);
    static Snapshort GetSnapshot(vector<UInt64> & partList);
  };
  class Web{};
};




#endif //  TRAN_MANAGER_HPP_ 
