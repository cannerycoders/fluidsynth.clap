//
// https://developer.apple.com/forums/thread/130855
// just iterate over all the files in the package
//
// codesign --sign 807CCBABA311632D43C4E01B0624F4C76ED9925B --force --timestamp \
//    --options runtime 
//    --entitlements ../build/entitlements.plist 
//    file