//
// https://developer.apple.com/forums/thread/130855
// just iterate over all the files in the package
//
// codesign --sign magic --force --timestamp \
//    --options runtime 
//    --entitlements ../build/entitlements.plist 
//    file