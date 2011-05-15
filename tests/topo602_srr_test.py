import topo602_base


class Test_Topo602_srr(topo602_base.Topo602):
  def __init__(self):
    topo602_base.Topo602.__init__(self,
                                  topo602_base.BINARY,
                                  topo602_base.REF_BINARY,
                                  topo602_base.REF_BINARY)


teardown = topo602_base.kill_all_instances
