HEADER
SCREEN  param
VERSION 1.0
LANGUAGE c
END

 FIELD    100 

 MASK_CHAR    _ 

 DISPLAY_MASK  [1,2] _(10) 
STRING  X(10)
MASK_VIDEO (REVERSE)

END

 FIELD    200 

 PROMPT [1,33]  "PARAMETERS  FILE" 
PROMPT_VIDEO (REVERSE)

END

 FIELD    300 

 MASK_CHAR    # 

 PROMPT [1,63]  "Date:" 

 DISPLAY_MASK  [1,69] ####/##/## 
DATE    YYYYFMMFDD
LOG_ATTR (REQUIRED,SHODUP)
PROMPT_VIDEO (REVERSE)

END

 FIELD    400 

 MASK_CHAR    _ 

 PROMPT [3,1]  "Company/District Name:" 

 DISPLAY_MASK  [3,24] _(50) 
STRING  X(50)
LOG_ATTR (REQUIRED,SHODUP)

END

 FIELD    500 

 MASK_CHAR    # 

 PROMPT [4,1]  "District Cost Center Number :" 

 DISPLAY_MASK  [4,31] #### 
NUMERIC   9999
LOG_ATTR (SUP_ZEROES,SHODUP)

END

 FIELD    600 

 MASK_CHAR    # 

 PROMPT [5,1]  "Warehouse Cost Center Number:" 

 DISPLAY_MASK  [5,31] #### 
NUMERIC   9999
LOG_ATTR (SUP_ZEROES,SHODUP)

END

 FIELD    700 

 MASK_CHAR    _ 

 PROMPT [6,1]  "Company or School District (C/S)    ?" 

 DISPLAY_MASK  [6,39] _ 
STRING  X
LOG_ATTR (VALIDITY,UCASE,SHODUP)

END

 FIELD    800 

 MASK_CHAR    _ 

 PROMPT [7,1]  "A/P System Present (Y/N)            ?" 

 DISPLAY_MASK  [7,39] _ 
BOOLEAN   B
LOG_ATTR (VALIDITY,UCASE,SHODUP)

END

 FIELD    900 

 MASK_CHAR    _ 

 PROMPT [8,1]  "Requisitioning System Present (Y/N) ?" 

 DISPLAY_MASK  [8,39] _ 
BOOLEAN   B
LOG_ATTR (UCASE,SHODUP)

END

 FIELD    1000 

 MASK_CHAR    _ 

 PROMPT [9,1]  "PO/Inventory Interface (Y/N)        ?" 

 DISPLAY_MASK  [9,39] _ 
BOOLEAN   B
LOG_ATTR (UCASE,SHODUP)

END

 FIELD    1100 

 MASK_CHAR    _ 

 PROMPT [10,1]  "Inventory System Present (Y/N)      ?" 

 DISPLAY_MASK  [10,39] _ 
BOOLEAN   B
LOG_ATTR (VALIDITY,UCASE,SHODUP)

END

 FIELD    1200 

 MASK_CHAR    _ 

 PROMPT [11,1]  "Fixed Assets System Present (Y/N)   ?" 

 DISPLAY_MASK  [11,39] _ 
BOOLEAN   B
LOG_ATTR (UCASE,SHODUP)

END

 FIELD    1300 

 MASK_CHAR    _ 

 PROMPT [12,1]  "A/R System Present (Y/N)            ?" 

 DISPLAY_MASK  [12,39] _ 
BOOLEAN   B
LOG_ATTR (UCASE,SHODUP)

END

 FIELD    1400 

 MASK_CHAR    _ 

 PROMPT [13,1]  "Cancel Pending POs at Year End (Y/N)?" 

 DISPLAY_MASK  [13,39] _ 
BOOLEAN   B
LOG_ATTR (UCASE,SHODUP)

END

 FIELD    1500 

 MASK_CHAR    _ 

 PROMPT [14,1]  "Move Budget Amts at Year End (Y/N)  ?" 

 DISPLAY_MASK  [14,39] _ 
BOOLEAN   B
LOG_ATTR (UCASE,SHODUP)

END

 FIELD    1600 

 MASK_CHAR    _ 

 PROMPT [15,1]  "More Than 1 Fund (Y/N)              ?" 

 DISPLAY_MASK  [15,39] _ 
BOOLEAN   B
LOG_ATTR (UCASE,SHODUP)

END

 FIELD    1700 

 MASK_CHAR    _ 

 PROMPT [16,1]  "G/L Master File Present (Y/N)       ?" 

 DISPLAY_MASK  [16,39] _ 
BOOLEAN   B
LOG_ATTR (UCASE,SHODUP)

END

 FIELD    1800 

 MASK_CHAR    _ 

 PROMPT [17,1]  "Consolidated Chequing (Y/N)         ?" 

 DISPLAY_MASK  [17,39] _ 
BOOLEAN   B
LOG_ATTR (UCASE,SHODUP)

END

 FIELD    1900 

 MASK_CHAR    _ 

 PROMPT [18,1]  "Distribute GST (Y/N)                ?" 

 DISPLAY_MASK  [18,39] _ 
BOOLEAN   B
LOG_ATTR (UCASE,SHODUP)

END

 FIELD    2000 

 MASK_CHAR    # 

 PROMPT [20,1]  "No of Periods         :" 

 DISPLAY_MASK  [20,25] ## 
NUMERIC   99
LOG_ATTR (REQUIRED,BOUNDS,SHODUP)
LOWER_BOUND  "12"
UPPER_BOUND  "13"

END

 FIELD    2100 

 MASK_CHAR    # 

 PROMPT [21,1]  "No of Pre Periods Open:" 

 DISPLAY_MASK  [21,25] ## 
NUMERIC   99
LOG_ATTR (VALIDITY,SHODUP)

END

 FIELD    2200 

 MASK_CHAR    # 

 PROMPT [22,1]  "Current Period        :" 

 DISPLAY_MASK  [22,25] ## 
NUMERIC   99
LOG_ATTR (REQUIRED,BOUNDS,SHODUP)
LOWER_BOUND  "01"
UPPER_BOUND  "13"

END

 FIELD    2300 

 MASK_CHAR    _ 

 PROMPT [7,54]  "Budget  Keys" 

 DISPLAY_MASK  [7,67] _ 
STRING  X

END

 FIELD    2400 

 MASK_CHAR    # 

 DISPLAY_MASK  [8,50] ## 
NUMERIC   99
LOG_ATTR (VALIDITY,SHODUP)

END

 FIELD    2500 

 MASK_CHAR    # 

 DISPLAY_MASK  [8,53] ## 
NUMERIC   99
LOG_ATTR (VALIDITY,SHODUP)

END

 FIELD    2600 

 MASK_CHAR    # 

 DISPLAY_MASK  [8,56] ## 
NUMERIC   99
LOG_ATTR (VALIDITY,SHODUP)

END

 FIELD    2700 

 MASK_CHAR    # 

 DISPLAY_MASK  [8,59] ## 
NUMERIC   99
LOG_ATTR (VALIDITY,SHODUP)

END

 FIELD    2800 

 MASK_CHAR    # 

 DISPLAY_MASK  [8,62] ## 
NUMERIC   99
LOG_ATTR (VALIDITY,SHODUP)

END

 FIELD    2900 

 MASK_CHAR    # 

 DISPLAY_MASK  [8,65] ## 
NUMERIC   99
LOG_ATTR (VALIDITY,SHODUP)

END

 FIELD    3000 

 MASK_CHAR    # 

 DISPLAY_MASK  [8,68] ## 
NUMERIC   99
LOG_ATTR (VALIDITY,SHODUP)

END

 FIELD    3100 

 MASK_CHAR    # 

 PROMPT [10,50]  "Cost Center Key: " 

 DISPLAY_MASK  [10,67] ## 
NUMERIC   99
LOG_ATTR (VALIDITY,SHODUP)

END

 FIELD    3200 

 MASK_CHAR    # 

 PROMPT [12,50]  "Purchasing Limit: " 

 DISPLAY_MASK  [12,68] #(7).## 
NUMERIC   9(7).99
LOG_ATTR (VALIDITY,SUP_ZEROES,SHODUP)

END

 FIELD    3300 

 MASK_CHAR    # 

 PROMPT [16,43]  "No of Days Due Default (PO): " 

 DISPLAY_MASK  [16,72] ## 
NUMERIC   99
LOG_ATTR (SHODUP)

END

 FIELD    3400 

 MASK_CHAR    # 

 PROMPT [17,43]  "No of Days Due Default (AP): " 

 DISPLAY_MASK  [17,72] ## 
NUMERIC   99
LOG_ATTR (SHODUP)

END

 FIELD    3500 

 MASK_CHAR    # 

 PROMPT [18,43]  "No of Days Due Default (AR): " 

 DISPLAY_MASK  [18,72] ## 
NUMERIC   99
LOG_ATTR (SHODUP)

END

 FIELD    3600 

 MASK_CHAR    _ 

 PROMPT [19,43]  "GST Registration Number: " 

 DISPLAY_MASK  [19,68] _(10) 
STRING  X(10)
LOG_ATTR (SHODUP)

END

 FIELD    3700 

 MASK_CHAR    _ 

 PROMPT [20,43]  "GST Field Default: " 

 DISPLAY_MASK  [20,62] _ 
STRING  X
LOG_ATTR (VALIDITY,UCASE,SHODUP)

END

 FIELD    3800 

 MASK_CHAR    _ 

 PROMPT [21,43]  "PST Field Default: " 

 DISPLAY_MASK  [21,62] _ 
STRING  X
LOG_ATTR (VALIDITY,UCASE,SHODUP)

END

 FIELD    3900 

 MASK_CHAR    _ 

 DISPLAY_MASK  [23,1] _(77) 
STRING  X(77)
MASK_VIDEO (REVERSE)

END

 FIELD    4000 

 MASK_CHAR    _ 

 DISPLAY_MASK  [23,79] _ 
STRING  X
LOG_ATTR (UCASE)

END

ENDFILE
