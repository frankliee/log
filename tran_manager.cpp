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

void TranService::WebServiceBehav(caf::event_based_actor * self) {

}

void TranService::GarbageCollectBehav(caf::event_based_actor * self){

}

void TranService::RecoveryFromLog() {

}

TranPtr TranAPIs::Local::CreateWriteTran(
    vector<pair<UInt64,UInt64>> & partList){
  TranService & ts = TranService::getInstance();
  auto tid = ts.TM.TId.fetch_add(1);
  TranPtr tran_ptr = make_shared<Tran>(tid,  kWrite,partList.size());
  LogManager::LogBegin(tid);
  for (auto i=0; i<partList.size();i++) {
    tran_ptr->StripList[i]->PartId = partList[i].first;
    tran_ptr->StripList[i]->Offset = partList[i].second;
    tran_ptr->StripList[i]->Pos = ts.TM.PosList[tran_ptr->StripList[i]->PartId].
        fetch_add(tran_ptr->StripList[i]->Offset);
    LogManager::LogWrite(tid,tran_ptr->StripList[i]->PartId,
                         tran_ptr->StripList[i]->Pos,
                         tran_ptr->StripList[i]->Offset);
  }
  do {
    tran_ptr->NextPtr = ts.TM.WriteHeadPtr.load();
  } while(!ts.TM.WriteHeadPtr.compare_exchange_weak(
      tran_ptr->NextPtr,tran_ptr));
  return tran_ptr;
}



