// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_HISTORY_URL_UTILS_H_
#define CHROME_BROWSER_HISTORY_URL_UTILS_H_

#include <string>

#include "chrome/browser/history/history_types.h"

namespace history {

// CanonicalURLStringCompare performs lexicographical comparison of two strings
// that represent valid URLs, so that if the pre-path (scheme, host, and port)
// parts are equal, then the path parts are compared by treating path components
// (delimited by "/") as separate tokens that form units of comparison.
// For example, let us compare |s1| and |s2|, with
//   |s1| = "http://www.google.com:80/base/test/ab/cd?query/stuff"
//   |s2| = "http://www.google.com:80/base/test-case/yz#ref/stuff"
// The pre-path parts "http://www.google.com:80/" match. We treat the paths as
//   |s1| => ["base", "test", "ab", "cd"]
//   |s2| => ["base", "test-case", "yz"]
// Components 1 "base" are identical. Components 2 yield "test" < "test-case",
// so we consider |s1| < |s2|, and return true. Note that naive string
// comparison would yield the opposite (|s1| > |s2|), since '/' > '-' in ASCII.
// Note that path can be terminated by "?query" or "#ref". The post-path parts
// are compared in an arbitrary (but consistent) way.
bool CanonicalURLStringCompare(const std::string& s1, const std::string& s2);

// Returns whether or not |url1| is a "URL prefix" of |url2|. Criteria:
// - Scheme, host, port: exact match required.
// - Path: treated as a list of path components (e.g., ["a", "bb"] for "/a/bb"),
//   and |url1|'s list must be a prefix of |url2|'s list.
// - Query and ref: ignored.
// Note that "http://www.google.com/test" is NOT a prefix of
// "http://www.google.com/testing", although "test" is a prefix of "testing".
bool UrlIsPrefix(const GURL& url1, const GURL& url2);

}  // namespace history

#endif  // CHROME_BROWSER_HISTORY_URL_UTILS_H_
