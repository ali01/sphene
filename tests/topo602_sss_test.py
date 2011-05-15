import topo602_base


class Test_Topo602_sss(topo602_base.Topo602):
  '''Topo602 sss'''
  def __init__(self):
    topo602_base.Topo602.__init__(self,
                                  topo602_base.BINARY,
                                  topo602_base.BINARY,
                                  topo602_base.BINARY)
