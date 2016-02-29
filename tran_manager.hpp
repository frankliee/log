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
#include <chrono>
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
const double kDefaulteLowerRate = 0;
const double kDefaulteUpperRate = 1000000;
const int kDefaultGCInterval = 2;
using GCAtom = caf::atom_constant<caf::atom("GC")>;

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
  char Type = kRead;
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
  ~Tran(){
    for (auto strip:StripList)
      delete strip;
  }
};
using TranPtr = shared_ptr<Tran>;

class Checkpoint{
 public:
  UInt64 MemoryPos;
  UInt64 HdfsPos;
};

class TranManager{
 public:
  atomic<UInt64> TId;
  atomic<TranPtr> WriteHead;
  atomic<TranPtr> ReadHead;
  atomic<int> CommitCount;
  atomic<int> AbortPreCount;
  unordered_map<UInt64,Checkpoint> CheckpointList;
  unordered_map<UInt64,atomic<UInt64>> PosList;
};

class TranService{
 public:
  TranManager TM;
  UInt16 Port = kDefaultTranPort;
  double UpperRate = 0;
  double LowerRate = 0;
  int GCInterval = kDefaultGCInterval;
  static TranService & getInstance(){
   static TranService trans_service;
   return trans_service;
  }
  static void setPort(UInt16 port) {
   TranService & ts = TranService::getInstance();
   ts.Port = port;
   }
  static void setUpperRate(double upperRate) {
    TranService & ts = TranService::getInstance();
    ts.UpperRate = upperRate;
  }
  static void setLowerRate(double lowerRate) {
    TranService & ts = TranService::getInstance();
    ts.LowerRate = lowerRate;
  }
  static void setGCInterval (int interval) {
    TranService &ts = TranService::getInstance();
    ts.GCInterval = interval;

  }
  static void Startup() {
   RecoveryFromLog();
   LogManager::Startup();
   pthread_t pid;
   pthread_create(&pid, NULL, WebServiceThread, NULL);
   pthread_create(&pid, NULL, GarbageCollectThread, NULL);
   pthread_create(&pid, NULL, LogDeleteThread, NULL);
   sleep(1);
  }
 private:
   static void RecoveryFromLog();
   static void * WebServiceThread(void * arg){
     caf::spawn(WebServiceBehav);
     caf::await_all_actors_done();
   }
   static void   WebServiceBehav(caf::event_based_actor * self);
   static void * GarbageCollectThread(void*);
   static void * LogDeleteThread(void*);

   TranService(){}
   TranService(const TranService & tran_service) {}
   TranService & operator = (const TranService & trans_service){}
};
struct SnapshotPart {
  UInt64 CheckpointPos;
  vector<Strip> StripList;
};
struct Snapshot {
  unordered_map<UInt64,UInt64> CheckpointList;
  vector<Strip> StripList;
};

class TranAPIs{
  class Local{
   public:
    static TranPtr CreateWriteTran(vector<pair<UInt64,UInt64>> & partList);
    static TranPtr CreateReadTran(vector<UInt64> & partList);
    inline static RetCode CommitWriteTran(TranPtr & tranPtr);
    inline static RetCode AbortWriteTran(TranPtr & tranPtr);
    inline static RetCode CommitReadTran(TranPtr & tranPtr);
    static Snapshot GetSnapshot(vector<UInt64> & partList);
  };
  class Web{};
};




#endif //  TRAN_MANAGER_HPP_ 
