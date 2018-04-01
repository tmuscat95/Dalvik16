javac *.java
dx --dex --output=classes.dex --min-sdk-version=38  $(find -type f -name "*.class")
dexdump -d classes.dex > classes.dump
dedex classes.dex
#rm *.class
./Dalvik16_Wrapper classes.dex
python removeDups.py
echo Everything Done. Exiting

