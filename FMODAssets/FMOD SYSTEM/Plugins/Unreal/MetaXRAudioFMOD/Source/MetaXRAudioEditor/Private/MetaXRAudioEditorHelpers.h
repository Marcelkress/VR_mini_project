// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

static FString GetSizeString(int64 size) {
  if (size <= (int64)0)
    return FString::Printf(TEXT("0 B"));
  else if (size < (int64)1024)
    return FString::Printf(TEXT("%lld B"), size);
  else if (size < 1024L * (int64)1024)
    return FString::Printf(TEXT("%lld KB"), size / 1024);
  else if (size < (int64)1024 * 1024 * 1024)
    return FString::Printf(TEXT("%lld MB"), size / (1024 * 1024));
  else if (size < (int64)1024 * 1024 * 1024 * 1024)
    return FString::Printf(TEXT("%lld GB"), size / ((int64)1024 * 1024 * 1024));
  else
    return FString::Printf(TEXT("%lld TB"), size / ((int64)1024 * 1024 * 1024 * 1024));
}
