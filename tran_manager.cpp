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
 * /log/trans_manager.cpp
 *
 *  Created on: 2016年2月25日
 *      Author: imdb
 *		   Email: 
 * 
 * Description:
 *
 */


#include "tran_manager.hpp"

TranManager TranService::TM;
UInt16 TranService::Port = kDefaultTranPort;
double TranService::LTranGCRate = kDefaulteLowerRate;
double TranService::UTranGCRate = kDefaulteUpperRate;
UInt32 TranService::TranGCTime = kDefaultGCInterval;


void TranService::WebAPIsBehav(caf::event_based_actor * self) {

}

void TranService::TranGCBehav(caf::event_based_actor * self){

}

void TranService::LogGCBehav(caf::event_based_actor * self) {

}

void TranService::RecoveryFromLog( ) {

}

void TranService::RecoveryFromCatalog() {

}

Tran * TranAPIs::Local::CreateWriteTran(vector<FixTupleStrip> & stripList ){

  if (stripList.size() == 0)
    return nullptr;
  UInt64 id = TranService::getId();
  Tran * tran_ptr = & TranService::getHead()->PayLoad;
  tran_ptr->setId(id);
  string log = LogService::LogBeginContent(id);
  for (auto i=0; i<stripList.size(); i++) {
    UInt64 partId = stripList[i].PartId;
    UInt64 pos = 0;
    UInt64 new_pos = 0;
    UInt32 offset = 0;
    do {
      pos = TranService::TM.PosList[partId].load();
      offset = stripList[i].SizeOfTuple * stripList[i].NumOfTuple;
      new_pos = pos + offset;
    } while(!TranService::TM.PosList[partId].compare_exchange_weak(pos, new_pos));
    tran_ptr->StripList.emplace_back(partId, pos, offset);
    log += LogService::LogWriteContent(id, partId, pos, offset);
  }
  LogService::LogAppend(log);
  TranService::TM.Count ++;
  return tran_ptr;
}
/*
Snapshot TranAPIs::Local::CreateReadTran(vector<UInt64> & partList){
  Snapshot snapshot;

  TranService & ts = TranService::getInstance();
  for (auto & part : partList) {
    snapshot.CheckpointList[part] = ts.TM.CheckpointList[part].MemoryPos;
  }
  auto ptr = ts.TM.WriteHead.load();
  auto rate = (double)ts.TM.CommitCount / (double)ts.TM.AbortPreCount;


  if (rate >= ts.LowerRate &&  rate <= ts.UpperRate) { // scan implement
    while (ptr !=  nullptr) {
      if (ptr->Visible == kVisible) {
        for (auto & item : ptr->StripList)
          snapshot.StripList.push_back(*item);
      }
      ptr = ptr->NextPtr;
    }
  } else if (rate < ts.LowerRate) { // simd implement

  } else if (rate > ts.UpperRate) { // simd implement

  }

  return snapshot;
}
  */

RetCode TranAPIs::Local::CommitWriteTran(Tran * tranPtr) {
  LogService::LogCommit(tranPtr->Id);
  tranPtr->IsVisible = true;
  TranService::TM.CommitCount ++;
  return 0;
}

RetCode TranAPIs::Local::AbortWriteTran(Tran * tranPtr) {
  LogService::LogAbort(tranPtr->Id);
  tranPtr->IsGarbage = true;
  return 0;
}

RetCode TranAPIs::Local::getSnapshot(vector<UInt64> & partList, Snapshot & snapshot){
  snapshot.clear();
  for (auto & partId:partList) {
    vector<Strip> stripList = {Strip(partId, 0, TranService::getMemCP(partId))};
    snapshot[partId] = stripList;
  }
  Node<Tran> * ptr = TranService::getHead();
  while (ptr != nullptr) {
    if (ptr->PayLoad.IsVisible)
      for (auto & strip : ptr->PayLoad.StripList) {
          if (strip.Pos >= snapshot[strip.PartId][0].Offset)
              snapshot[strip.PartId].push_back(strip);
        }
    ptr = ptr->Next;
  }
  return 0;
}

Checkpoint  TranAPIs::Local::CreateCPTran(UInt64 partId) {
  UInt64 id = TranService::getId();
  UInt64 old_mem_cp = TranService::getMemCP(partId);
  UInt64 new_mem_cp = old_mem_cp;
  UInt64 old_hdfs_cp = TranService::getHdfsCP(partId);
  UInt64 new_hdfs_cp = old_mem_cp;
  Node<Tran> * ptr = TranService::getHead();
  vector<Strip> buffer;
  while (ptr != nullptr) {
    if (ptr->PayLoad.IsVisible) {
      for (auto & strip : ptr->PayLoad.StripList) {
        if (strip.PartId == partId && strip.Pos >= old_mem_cp)
          buffer.push_back(strip);
      }
    }
    ptr = ptr->Next;
  }
  sort(buffer.begin(),buffer.end(),
       [](const Strip & a, const Strip & b){return a.Pos < b.Pos;});
  for (auto & strip : buffer) {
    if (strip.Pos != new_mem_cp)
      break;
    else
      new_mem_cp = strip.Pos + strip.Offset;
  }
  LogService::LogCP(id, partId, new_mem_cp);
  return Checkpoint(id, partId, new_mem_cp, old_mem_cp,new_mem_cp,old_mem_cp);
}

RetCode TranAPIs::Local::CommitCPTran(Checkpoint cp) {
  LogService::LogCP(cp.Id,cp.PartId,cp.NewMemPos);
  TranService::TM.MemCPList[cp.PartId] = cp.NewMemPos;
  TranService::TM.HdfsCPList[cp.PartId] = cp.NewHdfsPos;
  return 0;
}

RetCode TranAPIs::Local::AbortCPTran(Checkpoint cp) {
  LogService::LogAbort(cp.Id);
  TranService::TM.MemCPList[cp.PartId] = cp.NewMemPos;
  TranService::TM.HdfsCPList[cp.PartId] = cp.NewHdfsPos;
  return 0;
}


