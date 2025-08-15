/*
* Copyright (C) 2020 Flandreunx@outlook.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TEMP_CONFIG_MD_XBUTTON_H_
#define TEMP_CONFIG_MD_XBUTTON_H_

/// @xbuttonConfig
/////////////////////////////////

#define _TEMP_CONFIG_XBUTTON_FILTER_MS                        (10)
#define _TEMP_CONFIG_XBUTTON_DEFAULT_ACCESS_TH_COUNT          (10 / CONFIG_XBUTTON_FILTER_MS)
#define _TEMP_CONFIG_XBUTTON_DEFAULT_ACCESS_2_TH_COUNT        (300 / CONFIG_XBUTTON_FILTER_MS)
#define _TEMP_CONFIG_XBUTTON_DEFAULT_ACCESS_2_EMIT_TH_COUNT   ((CONFIG_XBUTTON_DEFAULT_ACCESS_2_TH_COUNT + 100) / CONFIG_XBUTTON_FILTER_MS)

#define _TEMP_CONFIG_XBUTTON_ROTENC_FILTER_MS                 (0)
#define _TEMP_CONFIG_XBUTTON_ROTENC_RELEASE_COUNT             (30 / CONFIG_XBUTTON_FILTER_MS)
#define _TEMP_CONFIG_XBUTTON_ROTENC_ENABLE                    (1)

#endif
