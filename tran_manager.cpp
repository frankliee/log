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

Tran * TranAPIs::Local::CreateWriteTran(vector<UInt64> & partList,
                                        vector<UInt32> & tupleNumList ){

   /*
    UInt64 id = TranService::TM.Id.fetch_add(1);
    Tran* tran_ptr = TranService::TM.WriteList.Insert();
    //cout << "s0" << endl;
    tran_ptr->Init(id, kWrite);
    //cout << "s1" << endl;
    string log = LogService::LogBeginContent(id);
    for (auto i=0; i<partList.size(); i++) {
        auto partid = partList[i].first;
        auto offset = partList[i].second;
        auto pos = TranService::TM.PosList[partid].fetch_add(offset);
        tran_ptr->StripList.emplace_back(partid, pos, offset);
        log += LogService::LogWriteContent(tran_ptr->Id,partid,pos,offset);
     }
    cout << TranService::TM.WriteList.ToString() << endl;
   // cout << "s2" << endl;
    LogService::LogAppend(log);
    TranService::TM.AbortPreCount ++;
   // cout << "s3" << endl;
    return tran_ptr;
     */
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

  return 0;
}

RetCode TranAPIs::Local::AbortWriteTran(Tran * tranPtr) {

  return 0;
}


RetCode TranAPIs::Local::getSnapshot(vector<UInt64> & partList, Snapshot & snapshot){

}

Checkpoint TranAPIs::Local::CreateCPTran(UInt64 partId) {

}

RetCode TranAPIs::Local::CommitCPTran(Checkpoint & cp) {

}

RetCode TranAPIs::Local::AbortCPTran(Checkpoint & cp) {

}


