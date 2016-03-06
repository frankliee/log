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
#include "lock_free.hpp"


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


struct Strip {
  UInt64 PartId;
  UInt64 Pos;
  UInt64 Offset;
  Strip(UInt64 partId, UInt64 pos, UInt64 offset):
  PartId(partId), Pos(pos), Offset(offset){}
};

struct Checkpoint{
  UInt64 MemoryPos;
  UInt64 HdfsPos;
};

class Tran{
 public:
  UInt64 Id = 0;
  int Type = kRead;
  bool IsVisible = false;
  bool IsGarbage = false;
  vector<Strip> StripList;
  Tran() {}
  void Init(UInt64 id, int type) {
    Id = id;
    Type = type;
  }
  void print() {
    cout <<"id:"<< Id << endl;
  }
  ~Tran() {
    cout << "tran destroy" << endl;
  }
  string ToString() {
    return "Id:"+to_string(Id)+",Type:"+to_string(Type)+
        ",IsVisible:"+to_string(IsVisible)+",isGarbage:"+to_string(IsGarbage);
  }
};

struct TranManager{
  atomic<UInt64> Id;
  atomic<UInt32> CommitCount;
  atomic<UInt32> AbortPreCount;
  unordered_map<UInt64,Checkpoint> CheckpointList;
  unordered_map<UInt64,atomic<UInt64>> PosList;
  LockFreeList<Tran> WriteList;
  //LockFreeList<pair<>>
};

class TranService{
 public:
  static TranManager TM;
  static UInt16 Port;
  static double UpperRate;
  static double LowerRate;
  static UInt32 GCInterval;
  static void setPort(UInt16 port) {
    Port = port;
   }
  static void setUpperRate(double upperRate) {
    UpperRate = upperRate;
  }
  static void setLowerRate(double lowerRate) {
    LowerRate = lowerRate;
  }
  static void setGCInterval (int interval) {
    GCInterval = interval;
  }
  static void Startup() {
   RecoveryFromCatalog();
   RecoveryFromLog();
   //pthread_t pid;
   //pthread_create(&pid, NULL, WebServiceThread, NULL);
   //pthread_create(&pid, NULL, GarbageCollectThread, NULL);
   //pthread_create(&pid, NULL, LogDeleteThread, NULL);

  }
 private:
   static void RecoveryFromLog();
   static void RecoveryFromCatalog();
   static void * WebServiceThread(void * arg){
     caf::spawn(WebServiceBehav);
     caf::await_all_actors_done();
   }
   static void   WebServiceBehav(caf::event_based_actor * self);
   static void * GarbageCollectThread(void*);
   static void * LogDeleteThread(void*);

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
 public:
  class Local{
   public:
    static Tran * CreateWriteTran(vector<pair<UInt64,UInt64>> & partList);
    static Snapshot CreateReadTran(vector<UInt64> & partList);
    static RetCode CommitWriteTran(Tran * tranPtr);
    static RetCode AbortWriteTran(Tran * tranPtr);
    static RetCode CommitReadTran(Tran * tranPtr);

  };
  class Web{};
};




#endif //  TRAN_MANAGER_HPP_ 
