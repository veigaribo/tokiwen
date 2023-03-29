import { EventEmitter } from "../event-emitter";

type MemoryEvents = {
  reset: [];
  write: [number, bigint];
  writeByte: [number, bigint];
};

export class Memory extends EventEmitter<MemoryEvents> {
  private readonly data: Uint8Array;

  constructor(private readonly size: number) {
    super();
    this.data = new Uint8Array(size);
  }

  reset() {
    for (var i = 0; i < this.size; ++i) {
      this.data[i] = 0;
    }

    this.emit("reset");
  }

  write(addr: number, value: bigint) {
    const originalValue = value;

    for (var i = addr; i < addr + 8; ++i) {
      const chunk = BigInt.asUintN(8, value);
      value >>= 8n;

      this.data[i] = Number(chunk);
      this.emit("writeByte", i, chunk);
    }

    this.emit("write", addr, originalValue);
  }

  read(addr: number): bigint {
    let result = 0n;

    for (var offset = 0; offset < 8; ++offset) {
      const chunk = BigInt(this.data[addr + offset]);

      result = (chunk << BigInt(8 * offset)) | result;
    }

    return BigInt.asIntN(64, result);
  }

  getSize(): number {
    return this.size;
  }

  load(data: Uint8Array) {
    for (var i = 0; i < data.length; ++i) {
      this.data[i] = data[i];
    }
  }
}
