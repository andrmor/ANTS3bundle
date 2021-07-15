TEMPLATE = subdirs
SUBDIRS = ants3 dispatcher psim

ants3.subdir      = src/ants3
dispatcher.subdir = src/dispatcher
psim.subdir       = src/psim

#CONFIG += ants2_WS          #if disabled, WebSockets are not compiled
