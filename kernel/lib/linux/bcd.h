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
 
#ifndef _BCD_H
#define _BCD_H

#define bcd2bin(x)	(((x) & 0x0F) + ((x) >> 4) * 10)
#define bin2bcd(x)	((((x) / 10) << 4) + (x) % 10)

#define bcd2numc_h(x)   (((x) >> 4) + 0x30)
#define bcd2numc_l(x)   (((x) & 0x0F) + 0x30)

#endif /* _BCD_H */
