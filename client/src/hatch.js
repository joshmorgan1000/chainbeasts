#!/usr/bin/env node
import('./wallet.js').then(async ({connect}) => {
    await connect();
    console.log('Creature hatched');
});
