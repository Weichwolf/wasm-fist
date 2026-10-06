# Audio ownership

Shared C11 owns sound events, sample decoding and final PCM mixing. Platform adapters own device
delivery only. No implementation exists yet. WI 0041 starts real audible events; WI 0044 completes
timing/device coverage. Original PCM bit identity is not required; absent audio fails.
