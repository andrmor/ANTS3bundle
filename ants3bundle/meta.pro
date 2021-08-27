TEMPLATE = subdirs
SUBDIRS = ants3 dispatcher psim lsim g4ants3

ants3.subdir      = src/ants3
dispatcher.subdir = src/dispatcher
psim.subdir       = src/psim
lsim.subdir       = src/lsim
g4ants3.subdir    = src/g4ants3

# to disable WebSockets, comment away these lines in ants3.pro and dispatcher.pro:
#CONFIG += ants3_FARM         #if commented away, WebSockets are not compiled and distributed (farm) functionality is disabled
# note that it cannot be done here (this is the Qt way...)
