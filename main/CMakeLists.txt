idf_component_register(
  SRCS
    "main.c"
    "pdm_mic/pdm_mic.c"
    "kissfft/kiss_fftr.c"
    "kissfft/kiss_fft.c"

  INCLUDE_DIRS 
    "pdm_mic"
    "kissfft"

)
