// Copyright (c) 2016 Adrien Guinet <adrien@guinet.me>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the <organization> nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PA_EXPORTS_H
#define PA_EXPORTS_H

#if defined _WIN32 || defined __CYGWIN__
  #define PA_HELPER_IMPORT __declspec(dllimport)
  #define PA_HELPER_EXPORT __declspec(dllexport)
  #define PA_HELPER_LOCAL
#else
  #define PA_HELPER_IMPORT __attribute__ ((visibility ("default")))
  #define PA_HELPER_EXPORT __attribute__ ((visibility ("default")))
  #define PA_HELPER_LOCAL  __attribute__ ((visibility ("hidden")))
#endif

#if defined(petanque_STATIC)
  #define PA_API
  #define PA_LOCAL
  #define PA_TEMPLATE_EXPIMP
#elif defined(petanque_EXPORTS)
  #define PA_API PA_HELPER_EXPORT
  #define PA_LOCAL PA_HELPER_LOCAL
  #define PA_TEMPLATE_EXPIMP
#else
  #define PA_API PA_HELPER_IMPORT
  #define PA_LOCAL PA_HELPER_LOCAL
  #define PA_TEMPLATE_EXPIMP extern
#endif

#endif
