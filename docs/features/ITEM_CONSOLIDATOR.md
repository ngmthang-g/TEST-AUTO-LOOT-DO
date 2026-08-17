# Feature — MAIN/CHILD Item Consolidator

## Roles

### MAIN
- move to train anchor;
- train;
- select CHILD target slot 1..6;
- initiate trade by click macro;
- receive/confirm;
- if bag becomes too full, stop receiving and run sell macro;
- return to anchor and resume train.

### CHILD
- move to train anchor;
- train;
- never sell;
- when free slots are low, wait for scheduler;
- only the selected CHILD stops train, moves to anchor, accepts trade, gives items and confirms;
- resume train after transaction.

## Eligibility

```text
CHILD wants transfer:
  bagReliable && freeSlots <= child_trigger_free_slots

MAIN needs sell:
  bagReliable && freeSlots < main_stop_free_slots
```

MAIN sell has higher priority than starting a new trade.

## Serialization

`transactionMutex` guards both trade and sell ownership of MAIN.

Other CHILD accounts remain training/waiting while one CHILD is trading.

## Target slots

At startup the user chooses CHILD windows in order. That order becomes:

```text
first CHILD  -> trade_invite_1
second CHILD -> trade_invite_2
...
sixth CHILD  -> trade_invite_6
```

This directly supports teams with 1..6 child accounts without hardcoding one party size.

## Dynamic transfer amount

`trade_give_items_child.macro` should use a `grid` step. Runtime caps grid clicks to:

```text
min(max_transfer_clicks_per_trade,
    MAIN.freeSlots - (main_stop_free_slots - 1))
```

with a minimum of 1 only when a trade has already been deemed eligible.

After the transaction all bags are rescanned by default. Stacking or untradeable items therefore do not rely on an assumed “one click = one free slot lost” model.

## Recovery

When the optional calibrated death signature matches an account:

```text
stop_train
-> revive_return
-> wait recovery
-> move_anchor
-> start_train
-> full rescan
```

The detector is intentionally disabled by default until a real death-screen sample is captured.
