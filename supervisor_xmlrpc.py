import xmlrpclib
import supervisor.xmlrpc
'''
use XML-RPC client to interact with supervisor process 
'''
# for unix domian socket
proxy = xmlrpclib.ServerProxy('http://127.0.0.1',transport=supervisor.xmlrpc.SupervisorTransport(serverurl='unix:///var/run//supervisor.sock'))
state =  proxy.supervisor.getState()
infos = proxy.supervisor.getAllProcessInfo()
for info in infos:
    process_name = info.get('group')
    r = proxy.supervisor.stopProcess(process_name)
    if r:
        r = proxy.supervisor.startProcess(process_name)