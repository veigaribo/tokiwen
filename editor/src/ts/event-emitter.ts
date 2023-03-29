type EventDefinitions = { [key: string]: [...any] };
type CallbackWith<A extends Array<any>> = (...args: A) => void;

type ListenerMap<D extends EventDefinitions> = {
  [P in keyof D]: CallbackWith<D[P]>[];
};

export class EventEmitter<D extends EventDefinitions> {
  private listeners: Partial<ListenerMap<D>> = {};

  public on<K extends keyof D>(event: K, callback: CallbackWith<D[K]>) {
    if (!this.listeners[event]) {
      this.listeners[event] = [];
    }

    this.listeners[event]!.push(callback);
  }

  protected emit<K extends keyof D>(event: K, ...params: D[K]) {
    for (const listener of this.listeners[event] || []) {
      listener(...params);
    }
  }
}
