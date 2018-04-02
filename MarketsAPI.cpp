//
// Copyright (c) 2016-2018, Karbo developers
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <jansson.h>

#include "MarketsAPI.h"

Markets::Markets(){
  this->api_status = false;
  this->res_status = false;
  this->time = 0;
  this->doErase();
}

Markets::~Markets(){
}

size_t Markets::WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
  size_t realsize = size * nmemb;
  Markets::MemoryStruct * mem = (struct MemoryStruct *) userp;
  mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL){
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}

char *Markets::client(){
  CURL *curl;
  CURLcode res;
  curl_slist *list = NULL;
  Markets::MemoryStruct chunk;
  chunk.memory = (char*) malloc(1);
  chunk.size = 0;
  const char api_host[] = "http://stats.karbovanets.org/services/karbo-markets/api.php?action=tickers";
  list = curl_slist_append(list, "Content-Type: application/json");
  this->api_status = false;
  curl = curl_easy_init();
  if(curl){
    curl_easy_setopt(curl, CURLOPT_URL, api_host);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_POST, 0L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Markets::WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
    res = curl_easy_perform(curl);
    curl_slist_free_all(list);
    if(res == CURLE_OK){
      if (chunk.size > 0) this->api_status = true;
	  } else {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}
    curl_easy_cleanup(curl);
  }
  return chunk.memory;
}

void Markets::doErase(){
  for (unsigned int i = 0; i < 16; i++){
    this->tickets[i].id = 0;
    memset(this->tickets[i].name, 0, 12);
    this->tickets[i].buy_active = 0;
    this->tickets[i].buy_effective = 0;
    this->tickets[i].sell_active = 0;
    this->tickets[i].sell_effective = 0;
    this->tickets[i].status = false;
  }
}

void Markets::doLoad(){
  json_t *json;
  json_error_t error;
  unsigned int pairs_all;
  json_t *tmg_status_obj;
  json_t *tmg_tickers_obj;
  json_t *tmg_time_obj;
  json_t *tmg_pairs_obj;
  json_t *tmg_pair_obj;
  json_t *ticker_id;
  json_t *ticker_name;
  json_t *ticker_buy_active;
  json_t *ticker_buy_effective;
  json_t *ticker_sell_active;
  json_t *ticker_sell_effective;
  json_t *ticker_status;
  this->res_status = false;
  json = json_loads(this->client(), 0, &error);
  if (this->api_status){
    if(json){
      if (json_is_object(json)){
        tmg_status_obj = json_object_get(json, "status");
        if (json_is_true(tmg_status_obj)){
          tmg_tickers_obj = json_object_get(json, "tickers");
          if (json_is_object(tmg_tickers_obj)){
            pairs_all = 0;
            this->doErase();
            tmg_time_obj = json_object_get(tmg_tickers_obj, "time");
            if (json_is_number(tmg_time_obj)){
              this->time = (int) json_real_value(tmg_time_obj);
            }
            tmg_pairs_obj = json_object_get(tmg_tickers_obj, "pairs");
            if (json_is_array(tmg_pairs_obj)){
              pairs_all = json_array_size(tmg_pairs_obj);
              if (pairs_all > 0){
                for (unsigned int p = 0; p < pairs_all; p++){
                  tmg_pair_obj = json_array_get(tmg_pairs_obj, p);
                  if (json_is_object(tmg_pair_obj)){
                    ticker_id = json_object_get(tmg_pair_obj, "id");
                    if (json_is_number(ticker_id)){
                      this->tickets[p].id = (int) json_number_value(ticker_id);
                      json_object_clear(ticker_id);
                    }
                    ticker_name = json_object_get(tmg_pair_obj, "name");
                    if (json_is_string(ticker_name)){
                      strncpy (this->tickets[p].name, json_string_value(ticker_name), sizeof(this->tickets[p].name));
                      json_object_clear(ticker_name);
                    }
                    ticker_buy_active = json_object_get(tmg_pair_obj, "buy_active");
                    if (json_is_number(ticker_buy_active)){
                      this->tickets[p].buy_active = json_number_value(ticker_buy_active);
                      json_object_clear(ticker_buy_active);
                    }
                    ticker_buy_effective = json_object_get(tmg_pair_obj, "buy_effective");
                    if (json_is_number(ticker_buy_effective)){
                      this->tickets[p].buy_effective = json_number_value(ticker_buy_effective);
                      json_object_clear(ticker_buy_effective);
                    }
                    ticker_sell_active = json_object_get(tmg_pair_obj, "sell_active");
                    if (json_is_number(ticker_sell_active)){
                      this->tickets[p].sell_active = json_number_value(ticker_sell_active);
                      json_object_clear(ticker_sell_active);
                    }
                    ticker_sell_effective = json_object_get(tmg_pair_obj, "sell_effective");
                    if (json_is_number(ticker_sell_effective)){
                      this->tickets[p].sell_effective = json_number_value(ticker_sell_effective);
                      json_object_clear(ticker_sell_effective);
                    }
                    ticker_status = json_object_get(tmg_pair_obj, "status");
                    if (json_is_boolean(ticker_status)){
                      if (json_is_true(ticker_status)){
                        this->tickets[p].status = true;
                      } else {
                        this->tickets[p].status = false;
                      }
                      json_object_clear(ticker_status);
                    }
                    json_object_clear(tmg_pair_obj);
                  }
                }
              }
            }
          }
          this->res_status = true;
        }
      }
    }
  }
}

bool Markets::getStatus(){
  return this->res_status;
}
