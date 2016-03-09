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
#include <functional>
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
using std::function;

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


struct FixTupleStrip {
  UInt64 PartId;
  UInt64 Pos;
  UInt32 SizeOfTuple;
  UInt32 NumOfTuple;
};

struct Strip {
  UInt64 PartId;
  UInt64 Pos;
  UInt32 Offset;
  Strip(){}
  Strip(UInt64 partId, UInt64 pos, UInt32 offset):
    PartId(partId), Pos(pos), Offset(offset){}
  string ToString() {
    return "PartId:"+to_string(PartId)+",Pos:"+to_string(Pos)+
        ",Offset:"+to_string(Offset)+"\n";
   }

  static void Map(vector<Strip> & input, map<UInt64,vector<Strip>> & output) {
    output.clear();
    for (auto & strip : input) {
      if (output.find(strip.PartId) != output.end())
        output[strip.PartId].push_back(strip);
      else
        output[strip.PartId] =  vector<Strip>();
    }
  }

  static void Sort(vector<Strip> & input) {
    sort(input.begin(), input.end(),
         [](const Strip & a, const Strip & b){ return a.Pos < b.Pos; });
  }

  static void Merge(vector<Strip> & input) {
      vector<Strip> buffer(input);
      input.clear();
      if (buffer.size() == 0 )return ;
      if (buffer.size() == 1) {input.push_back(buffer[0]);}
      UInt64 begin = buffer[0].Pos;
      UInt64 end = buffer[0].Pos + buffer[0].Offset;
      for (auto i=1; i<buffer.size();i++){
          if (end >= buffer[i].Pos)
            end = buffer[i].Pos + buffer[i].Offset;
          else {
            input.push_back(Strip(buffer[0].PartId, begin, end - begin));
            begin = buffer[i].Pos;
            end = buffer[i].Pos + buffer[i].Offset;
              }
       }
      input.push_back(Strip(buffer[0].PartId, begin, end -begin));
  }
  static void Filter(vector<Strip> & input, function<bool(Strip &)> fuc) {
     vector<Strip> buffer(input);
     input.clear();
     for (auto & strip : buffer)
       if (fuc(strip))
         input.push_back(strip);
  }
};


class Tran{
 public:
  UInt64 Id = 0;
  //int Type = kRead;
  bool IsVisible = false;
  bool IsGarbage = false;
  vector<Strip> StripList;
  Tran() {}
  ~Tran() {}
  void setId(UInt64 id){ Id = id;}
  string ToString() {
    string stripList = "striplist:\n";
    for (auto & strip:StripList)
        stripList += strip.ToString();
    return "Id:"+to_string(Id)+",IsVisible:"+to_string(IsVisible)+
        ",isGarbage:"+to_string(IsGarbage)+"\n"+stripList;
  }
};

struct TranManager{
  atomic<UInt64> Id;
  atomic<UInt32> CommitCount;
  atomic<UInt32> Count;
  unordered_map<UInt64, UInt64> MemCPList;
  unordered_map<UInt64, UInt64> HdfsCPList;
  unordered_map<UInt64, atomic<UInt64>> PosList;
  LockFreeList<Tran> WriteTranList;
};

class TranService{
 public:
  static TranManager TM;
  static UInt16 Port;
  static double UTranGCRate;
  static double LTranGCRate;
  static UInt32 TranGCTime;
  static void setPort(UInt16 port) {
    Port = port;
   }
  static void setUpperRate(double uTranGCRate) {
    UTranGCRate = uTranGCRate;
  }
  static void setLowerRate(double lTranGCRate) {
    LTranGCRate = lTranGCRate;
  }
  static void setGCTime (int time) {
    TranGCTime = time;
  }
  static void Startup() {
   RecoveryFromCatalog();
   RecoveryFromLog();
   caf::spawn(WebAPIsBehav);
  }
  static UInt64 getId() {
    return TM.Id.fetch_add(1);
  }
  static Node<Tran> * getHead() {
    return TM.WriteTranList.Head.load();
  }
  static UInt64 getMemCP(UInt64 partId){
    return TM.MemCPList[partId];
  }
  static UInt64 getHdfsCP(UInt64 partId) {
    return TM.HdfsCPList[partId];
  }
 private:
   static void RecoveryFromLog();
   static void RecoveryFromCatalog();
   static void WebAPIsBehav(caf::event_based_actor * self);
   static void TranGCBehav(caf::event_based_actor * self);
   static void LogGCBehav(caf::event_based_actor * self);
};

using Snapshot = map<UInt64, vector<Strip>>;

class Checkpoint{
 public:
  UInt64 Id;
  UInt64 PartId;
  UInt64 NewMemPos;
  UInt64 OldMemPos;
  UInt64 NewHdfsPos;
  UInt64 OldHPos;
  vector<Strip> MergeList;
  Checkpoint() {}
  Checkpoint(UInt64) {}
  bool IsVaild() { return NewMemPos > OldMemPos;}
  Checkpoint(UInt64 id,UInt64 partId,
             UInt64 newMPos,UInt64 oldMPos,UInt64 newHPos,UInt64 oldHPos):
             Id(id),PartId(partId),
             NewMemPos(newMPos),OldMemPos(oldMPos),NewHdfsPos(newHPos),OldHPos(oldHPos){}
};

class TranAPIs{
 public:
  class Local{
   public:
    static Tran * CreateWriteTran(vector<FixTupleStrip> & stripList);
    static RetCode CommitWriteTran(Tran * tranPtr);
    static RetCode AbortWriteTran(Tran * tranPtr);

    static RetCode getSnapshot(vector<UInt64> & partList, Snapshot & snapshot);

    static Checkpoint  CreateCPTran(UInt64 partId);
    static RetCode CommitCPTran(Checkpoint cp);
    static RetCode AbortCPTran(Checkpoint cp);

  };
  class Web{};
};




#endif //  TRAN_MANAGER_HPP_ 
