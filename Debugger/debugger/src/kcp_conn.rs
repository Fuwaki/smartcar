use std::{
    io::Write,
    net::{SocketAddr, ToSocketAddrs, UdpSocket},
    rc::Rc, sync::mpsc::TryIter,
};

use bytes::Bytes;
use esp_idf_hal::task::current;
use kcp::Kcp;

struct SocketWrite {
    socket: Rc<UdpSocket>,
    peer: SocketAddr,
}
impl Write for SocketWrite {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        self.socket.send_to(buf, &self.peer)
    }
    fn flush(&mut self) -> std::io::Result<()> {
        std::io::Result::Ok(())
    }
}
impl SocketWrite {
    fn new(socket: Rc<UdpSocket>, peer: SocketAddr) -> Self {
        Self { socket, peer }
    }
}
pub struct KcpConn {
    socket: Rc<UdpSocket>,
    kcp: Kcp<SocketWrite>,
    buf:bytes::BytesMut,
}
impl KcpConn {
    pub fn new(address: impl ToSocketAddrs, peer: impl ToSocketAddrs) -> anyhow::Result<Self> {
        let address = address.to_socket_addrs()?.next().unwrap();
        let socket = UdpSocket::bind(address)?;
        socket.set_nonblocking(true)?;
        let socket = Rc::new(socket);
        let peer = peer.to_socket_addrs()?.next().unwrap();
        let kcp = Kcp::new(114514, SocketWrite::new(socket.clone(), peer));
        Ok(Self { socket, kcp ,buf:bytes::BytesMut::with_capacity(512)})
    }
    pub fn mode() {}
    pub fn send(mut self,data: bytes::Bytes) {
        self.kcp.send(&data);
    }
    pub fn recv(mut self,data: &mut bytes::BytesMut) -> anyhow::Result<usize> {
        let s=self.kcp.recv(data)?;
        Ok(s)
    }
    pub fn update(mut self)->anyhow::Result<()>{
//update
        while let Ok((size,_))=self.socket.recv_from(&mut self.buf){
            let size=self.kcp.input(&mut self.buf[..size])?;
        }
        Ok(())

    }
}
