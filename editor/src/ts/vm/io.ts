import { EventEmitter } from "../event-emitter";

type IoEvents = {
  buffered: []; // write but no read
  waiting: []; // read but no write
};

type ResolveFn = (value: bigint) => void;

export abstract class IO extends EventEmitter<IoEvents> {
  private readonly queue: ResolveFn[] = [];
  private readonly buffer: bigint[] = [];

  public read(): Promise<bigint> {
    if (this.buffer.length > 0) {
      return Promise.resolve(this.buffer.shift()!);
    }

    return new Promise((resolve) => {
      this.queue.push(resolve);
      this.emit("waiting");
    });
  }

  public write(value: bigint): void {
    if (this.queue.length > 0) {
      const resolver = this.queue.shift()!;
      resolver(value);
    } else {
      this.buffer.push(value);
      this.emit("buffered");
    }
  }
}

export class Input extends IO {}
export class Output extends IO {}
