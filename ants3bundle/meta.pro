TEMPLATE = subdirs
SUBDIRS = ants3 dispatcher demo lsim g4ants3 g4inspector

ants3.subdir       = src/ants3
dispatcher.subdir  = src/dispatcher
demo.subdir        = src/demo
lsim.subdir        = src/lsim
g4ants3.subdir     = src/g4ants3
g4inspector.subdir = src/g4inspector

# to disable WebSockets, comment away these lines in ants3.pro and dispatcher.pro:
#CONFIG += ants3_FARM         #if commented away, WebSockets are not compiled and distributed (farm) functionality is disabled
# note that it cannot be done here (this is the Qt way...)
